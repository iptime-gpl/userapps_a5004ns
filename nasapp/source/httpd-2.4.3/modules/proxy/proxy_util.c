/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Utility routines for Apache proxy */
#include "mod_proxy.h"
#include "ap_mpm.h"
#include "scoreboard.h"
#include "apr_version.h"
#include "apr_hash.h"
#include "proxy_util.h"

#if APR_HAVE_UNISTD_H
#include <unistd.h>         /* for getpid() */
#endif

#if (APR_MAJOR_VERSION < 1)
#undef apr_socket_create
#define apr_socket_create apr_socket_create_ex
#endif

APLOG_USE_MODULE(proxy);

/*
 * Opaque structure containing target server info when
 * using a forward proxy.
 * Up to now only used in combination with HTTP CONNECT.
 */
typedef struct {
    int          use_http_connect; /* Use SSL Tunneling via HTTP CONNECT */
    const char   *target_host;     /* Target hostname */
    apr_port_t   target_port;      /* Target port */
    const char   *proxy_auth;      /* Proxy authorization */
} forward_info;

/* Keep synced with mod_proxy.h! */
static struct wstat {
    unsigned int bit;
    char flag;
    const char *name;
} wstat_tbl[] = {
    {PROXY_WORKER_INITIALIZED,   PROXY_WORKER_INITIALIZED_FLAG,   "Init "},
    {PROXY_WORKER_IGNORE_ERRORS, PROXY_WORKER_IGNORE_ERRORS_FLAG, "Ign "},
    {PROXY_WORKER_DRAIN,         PROXY_WORKER_DRAIN_FLAG,         "Drn "},
    {PROXY_WORKER_IN_SHUTDOWN,   PROXY_WORKER_IN_SHUTDOWN_FLAG,   "Shut "},
    {PROXY_WORKER_DISABLED,      PROXY_WORKER_DISABLED_FLAG,      "Dis "},
    {PROXY_WORKER_STOPPED,       PROXY_WORKER_STOPPED_FLAG,       "Stop "},
    {PROXY_WORKER_IN_ERROR,      PROXY_WORKER_IN_ERROR_FLAG,      "Err "},
    {PROXY_WORKER_HOT_STANDBY,   PROXY_WORKER_HOT_STANDBY_FLAG,   "Stby "},
    {PROXY_WORKER_FREE,          PROXY_WORKER_FREE_FLAG,          "Free "},
    {0x0, '\0', NULL}
};

/* Global balancer counter */
int PROXY_DECLARE_DATA proxy_lb_workers = 0;
static int lb_workers_limit = 0;
const apr_strmatch_pattern PROXY_DECLARE_DATA *ap_proxy_strmatch_path;
const apr_strmatch_pattern PROXY_DECLARE_DATA *ap_proxy_strmatch_domain;

static int proxy_match_ipaddr(struct dirconn_entry *This, request_rec *r);
static int proxy_match_domainname(struct dirconn_entry *This, request_rec *r);
static int proxy_match_hostname(struct dirconn_entry *This, request_rec *r);
static int proxy_match_word(struct dirconn_entry *This, request_rec *r);

APR_IMPLEMENT_OPTIONAL_HOOK_RUN_ALL(proxy, PROXY, int, create_req,
                                   (request_rec *r, request_rec *pr), (r, pr),
                                   OK, DECLINED)

PROXY_DECLARE(apr_status_t) ap_proxy_strncpy(char *dst, const char *src,
                                             apr_size_t dlen)
{
    char *thenil;
    apr_size_t thelen;

    thenil = apr_cpystrn(dst, src, dlen);
    thelen = thenil - dst;
    /* Assume the typical case is smaller copying into bigger
       so we have a fast return */
    if ((thelen < dlen-1) || ((strlen(src)) == thelen)) {
        return APR_SUCCESS;
    }
    /* XXX: APR_ENOSPACE would be better */
    return APR_EGENERAL;
}

/* already called in the knowledge that the characters are hex digits */
PROXY_DECLARE(int) ap_proxy_hex2c(const char *x)
{
    int i;

#if !APR_CHARSET_EBCDIC
    int ch = x[0];

    if (apr_isdigit(ch)) {
        i = ch - '0';
    }
    else if (apr_isupper(ch)) {
        i = ch - ('A' - 10);
    }
    else {
        i = ch - ('a' - 10);
    }
    i <<= 4;

    ch = x[1];
    if (apr_isdigit(ch)) {
        i += ch - '0';
    }
    else if (apr_isupper(ch)) {
        i += ch - ('A' - 10);
    }
    else {
        i += ch - ('a' - 10);
    }
    return i;
#else /*APR_CHARSET_EBCDIC*/
    /*
     * we assume that the hex value refers to an ASCII character
     * so convert to EBCDIC so that it makes sense locally;
     *
     * example:
     *
     * client specifies %20 in URL to refer to a space char;
     * at this point we're called with EBCDIC "20"; after turning
     * EBCDIC "20" into binary 0x20, we then need to assume that 0x20
     * represents an ASCII char and convert 0x20 to EBCDIC, yielding
     * 0x40
     */
    char buf[1];

    if (1 == sscanf(x, "%2x", &i)) {
        buf[0] = i & 0xFF;
        ap_xlate_proto_from_ascii(buf, 1);
        return buf[0];
    }
    else {
        return 0;
    }
#endif /*APR_CHARSET_EBCDIC*/
}

PROXY_DECLARE(void) ap_proxy_c2hex(int ch, char *x)
{
#if !APR_CHARSET_EBCDIC
    int i;

    x[0] = '%';
    i = (ch & 0xF0) >> 4;
    if (i >= 10) {
        x[1] = ('A' - 10) + i;
    }
    else {
        x[1] = '0' + i;
    }

    i = ch & 0x0F;
    if (i >= 10) {
        x[2] = ('A' - 10) + i;
    }
    else {
        x[2] = '0' + i;
    }
#else /*APR_CHARSET_EBCDIC*/
    static const char ntoa[] = { "0123456789ABCDEF" };
    char buf[1];

    ch &= 0xFF;

    buf[0] = ch;
    ap_xlate_proto_to_ascii(buf, 1);

    x[0] = '%';
    x[1] = ntoa[(buf[0] >> 4) & 0x0F];
    x[2] = ntoa[buf[0] & 0x0F];
    x[3] = '\0';
#endif /*APR_CHARSET_EBCDIC*/
}

/*
 * canonicalise a URL-encoded string
 */

/*
 * Convert a URL-encoded string to canonical form.
 * It decodes characters which need not be encoded,
 * and encodes those which must be encoded, and does not touch
 * those which must not be touched.
 */
PROXY_DECLARE(char *)ap_proxy_canonenc(apr_pool_t *p, const char *x, int len,
                                       enum enctype t, int forcedec,
                                       int proxyreq)
{
    int i, j, ch;
    char *y;
    char *allowed;  /* characters which should not be encoded */
    char *reserved; /* characters which much not be en/de-coded */

/*
 * N.B. in addition to :@&=, this allows ';' in an http path
 * and '?' in an ftp path -- this may be revised
 *
 * Also, it makes a '+' character in a search string reserved, as
 * it may be form-encoded. (Although RFC 1738 doesn't allow this -
 * it only permits ; / ? : @ = & as reserved chars.)
 */
    if (t == enc_path) {
        allowed = "~$-_.+!*'(),;:@&=";
    }
    else if (t == enc_search) {
        allowed = "$-_.!*'(),;:@&=";
    }
    else if (t == enc_user) {
        allowed = "$-_.+!*'(),;@&=";
    }
    else if (t == enc_fpath) {
        allowed = "$-_.+!*'(),?:@&=";
    }
    else {            /* if (t == enc_parm) */
        allowed = "$-_.+!*'(),?/:@&=";
    }

    if (t == enc_path) {
        reserved = "/";
    }
    else if (t == enc_search) {
        reserved = "+";
    }
    else {
        reserved = "";
    }

    y = apr_palloc(p, 3 * len + 1);

    for (i = 0, j = 0; i < len; i++, j++) {
/* always handle '/' first */
        ch = x[i];
        if (strchr(reserved, ch)) {
            y[j] = ch;
            continue;
        }
/*
 * decode it if not already done. do not decode reverse proxied URLs
 * unless specifically forced
 */
        if ((forcedec || (proxyreq && proxyreq != PROXYREQ_REVERSE)) && ch == '%') {
            if (!apr_isxdigit(x[i + 1]) || !apr_isxdigit(x[i + 2])) {
                return NULL;
            }
            ch = ap_proxy_hex2c(&x[i + 1]);
            i += 2;
            if (ch != 0 && strchr(reserved, ch)) {  /* keep it encoded */
                ap_proxy_c2hex(ch, &y[j]);
                j += 2;
                continue;
            }
        }
/* recode it, if necessary */
        if (!apr_isalnum(ch) && !strchr(allowed, ch)) {
            ap_proxy_c2hex(ch, &y[j]);
            j += 2;
        }
        else {
            y[j] = ch;
        }
    }
    y[j] = '\0';
    return y;
}

/*
 * Parses network-location.
 *    urlp           on input the URL; on output the path, after the leading /
 *    user           NULL if no user/password permitted
 *    password       holder for password
 *    host           holder for host
 *    port           port number; only set if one is supplied.
 *
 * Returns an error string.
 */
PROXY_DECLARE(char *)
     ap_proxy_canon_netloc(apr_pool_t *p, char **const urlp, char **userp,
            char **passwordp, char **hostp, apr_port_t *port)
{
    char *addr, *scope_id, *strp, *host, *url = *urlp;
    char *user = NULL, *password = NULL;
    apr_port_t tmp_port;
    apr_status_t rv;

    if (url[0] != '/' || url[1] != '/') {
        return "Malformed URL";
    }
    host = url + 2;
    url = strchr(host, '/');
    if (url == NULL) {
        url = "";
    }
    else {
        *(url++) = '\0';    /* skip seperating '/' */
    }

    /* find _last_ '@' since it might occur in user/password part */
    strp = strrchr(host, '@');

    if (strp != NULL) {
        *strp = '\0';
        user = host;
        host = strp + 1;

/* find password */
        strp = strchr(user, ':');
        if (strp != NULL) {
            *strp = '\0';
            password = ap_proxy_canonenc(p, strp + 1, strlen(strp + 1), enc_user, 1, 0);
            if (password == NULL) {
                return "Bad %-escape in URL (password)";
            }
        }

        user = ap_proxy_canonenc(p, user, strlen(user), enc_user, 1, 0);
        if (user == NULL) {
            return "Bad %-escape in URL (username)";
        }
    }
    if (userp != NULL) {
        *userp = user;
    }
    if (passwordp != NULL) {
        *passwordp = password;
    }

    /*
     * Parse the host string to separate host portion from optional port.
     * Perform range checking on port.
     */
    rv = apr_parse_addr_port(&addr, &scope_id, &tmp_port, host, p);
    if (rv != APR_SUCCESS || addr == NULL || scope_id != NULL) {
        return "Invalid host/port";
    }
    if (tmp_port != 0) { /* only update caller's port if port was specified */
        *port = tmp_port;
    }

    ap_str_tolower(addr); /* DNS names are case-insensitive */

    *urlp = url;
    *hostp = addr;

    return NULL;
}

PROXY_DECLARE(int) ap_proxyerror(request_rec *r, int statuscode, const char *message)
{
    const char *uri = ap_escape_html(r->pool, r->uri);
    apr_table_setn(r->notes, "error-notes",
        apr_pstrcat(r->pool,
            "The proxy server could not handle the request <em><a href=\"",
            uri, "\">", ap_escape_html(r->pool, r->method), "&nbsp;", uri,
            "</a></em>.<p>\n"
            "Reason: <strong>", ap_escape_html(r->pool, message),
            "</strong></p>",
            NULL));

    /* Allow "error-notes" string to be printed by ap_send_error_response() */
    apr_table_setn(r->notes, "verbose-error-to", "*");

    r->status_line = apr_psprintf(r->pool, "%3.3u Proxy Error", statuscode);
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00898) "%s returned by %s", message,
                  r->uri);
    return statuscode;
}

static const char *
     proxy_get_host_of_request(request_rec *r)
{
    char *url, *user = NULL, *password = NULL, *err, *host;
    apr_port_t port;

    if (r->hostname != NULL) {
        return r->hostname;
    }

    /* Set url to the first char after "scheme://" */
    if ((url = strchr(r->uri, ':')) == NULL || url[1] != '/' || url[2] != '/') {
        return NULL;
    }

    url = apr_pstrdup(r->pool, &url[1]);    /* make it point to "//", which is what proxy_canon_netloc expects */

    err = ap_proxy_canon_netloc(r->pool, &url, &user, &password, &host, &port);

    if (err != NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00899) "%s", err);
    }

    r->hostname = host;

    return host;        /* ought to return the port, too */
}

/* Return TRUE if addr represents an IP address (or an IP network address) */
PROXY_DECLARE(int) ap_proxy_is_ipaddr(struct dirconn_entry *This, apr_pool_t *p)
{
    const char *addr = This->name;
    long ip_addr[4];
    int i, quads;
    long bits;

    /*
     * if the address is given with an explicit netmask, use that
     * Due to a deficiency in apr_inet_addr(), it is impossible to parse
     * "partial" addresses (with less than 4 quads) correctly, i.e.
     * 192.168.123 is parsed as 192.168.0.123, which is not what I want.
     * I therefore have to parse the IP address manually:
     * if (proxy_readmask(This->name, &This->addr.s_addr, &This->mask.s_addr) == 0)
     * addr and mask were set by proxy_readmask()
     * return 1;
     */

    /*
     * Parse IP addr manually, optionally allowing
     * abbreviated net addresses like 192.168.
     */

    /* Iterate over up to 4 (dotted) quads. */
    for (quads = 0; quads < 4 && *addr != '\0'; ++quads) {
        char *tmp;

        if (*addr == '/' && quads > 0) {  /* netmask starts here. */
            break;
        }

        if (!apr_isdigit(*addr)) {
            return 0;       /* no digit at start of quad */
        }

        ip_addr[quads] = strtol(addr, &tmp, 0);

        if (tmp == addr) {  /* expected a digit, found something else */
            return 0;
        }

        if (ip_addr[quads] < 0 || ip_addr[quads] > 255) {
            /* invalid octet */
            return 0;
        }

        addr = tmp;

        if (*addr == '.' && quads != 3) {
            ++addr;     /* after the 4th quad, a dot would be illegal */
        }
    }

    for (This->addr.s_addr = 0, i = 0; i < quads; ++i) {
        This->addr.s_addr |= htonl(ip_addr[i] << (24 - 8 * i));
    }

    if (addr[0] == '/' && apr_isdigit(addr[1])) {   /* net mask follows: */
        char *tmp;

        ++addr;

        bits = strtol(addr, &tmp, 0);

        if (tmp == addr) {   /* expected a digit, found something else */
            return 0;
        }

        addr = tmp;

        if (bits < 0 || bits > 32) { /* netmask must be between 0 and 32 */
            return 0;
        }

    }
    else {
        /*
         * Determine (i.e., "guess") netmask by counting the
         * number of trailing .0's; reduce #quads appropriately
         * (so that 192.168.0.0 is equivalent to 192.168.)
         */
        while (quads > 0 && ip_addr[quads - 1] == 0) {
            --quads;
        }

        /* "IP Address should be given in dotted-quad form, optionally followed by a netmask (e.g., 192.168.111.0/24)"; */
        if (quads < 1) {
            return 0;
        }

        /* every zero-byte counts as 8 zero-bits */
        bits = 8 * quads;

        if (bits != 32) {     /* no warning for fully qualified IP address */
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00900)
                         "Warning: NetMask not supplied with IP-Addr; guessing: %s/%ld",
                         inet_ntoa(This->addr), bits);
        }
    }

    This->mask.s_addr = htonl(APR_INADDR_NONE << (32 - bits));

    if (*addr == '\0' && (This->addr.s_addr & ~This->mask.s_addr) != 0) {
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00901)
                     "Warning: NetMask and IP-Addr disagree in %s/%ld",
                     inet_ntoa(This->addr), bits);
        This->addr.s_addr &= This->mask.s_addr;
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00902)
                     "         Set to %s/%ld", inet_ntoa(This->addr), bits);
    }

    if (*addr == '\0') {
        This->matcher = proxy_match_ipaddr;
        return 1;
    }
    else {
        return (*addr == '\0'); /* okay iff we've parsed the whole string */
    }
}

/* Return TRUE if addr represents an IP address (or an IP network address) */
static int proxy_match_ipaddr(struct dirconn_entry *This, request_rec *r)
{
    int i, ip_addr[4];
    struct in_addr addr, *ip;
    const char *host = proxy_get_host_of_request(r);

    if (host == NULL) {   /* oops! */
       return 0;
    }

    memset(&addr, '\0', sizeof addr);
    memset(ip_addr, '\0', sizeof ip_addr);

    if (4 == sscanf(host, "%d.%d.%d.%d", &ip_addr[0], &ip_addr[1], &ip_addr[2], &ip_addr[3])) {
        for (addr.s_addr = 0, i = 0; i < 4; ++i) {
            /* ap_proxy_is_ipaddr() already confirmed that we have
             * a valid octet in ip_addr[i]
             */
            addr.s_addr |= htonl(ip_addr[i] << (24 - 8 * i));
        }

        if (This->addr.s_addr == (addr.s_addr & This->mask.s_addr)) {
#if DEBUGGING
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00903)
                         "1)IP-Match: %s[%s] <-> ", host, inet_ntoa(addr));
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00904)
                         "%s/", inet_ntoa(This->addr));
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00905)
                         "%s", inet_ntoa(This->mask));
#endif
            return 1;
        }
#if DEBUGGING
        else {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00906)
                         "1)IP-NoMatch: %s[%s] <-> ", host, inet_ntoa(addr));
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00907)
                         "%s/", inet_ntoa(This->addr));
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00908)
                         "%s", inet_ntoa(This->mask));
        }
#endif
    }
    else {
        struct apr_sockaddr_t *reqaddr;

        if (apr_sockaddr_info_get(&reqaddr, host, APR_UNSPEC, 0, 0, r->pool)
            != APR_SUCCESS) {
#if DEBUGGING
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00909)
             "2)IP-NoMatch: hostname=%s msg=Host not found", host);
#endif
            return 0;
        }

        /* Try to deal with multiple IP addr's for a host */
        /* FIXME: This needs to be able to deal with IPv6 */
        while (reqaddr) {
            ip = (struct in_addr *) reqaddr->ipaddr_ptr;
            if (This->addr.s_addr == (ip->s_addr & This->mask.s_addr)) {
#if DEBUGGING
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00910)
                             "3)IP-Match: %s[%s] <-> ", host, inet_ntoa(*ip));
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00911)
                             "%s/", inet_ntoa(This->addr));
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00912)
                             "%s", inet_ntoa(This->mask));
#endif
                return 1;
            }
#if DEBUGGING
            else {
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00913)
                             "3)IP-NoMatch: %s[%s] <-> ", host, inet_ntoa(*ip));
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00914)
                             "%s/", inet_ntoa(This->addr));
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, APLOGNO(00915)
                             "%s", inet_ntoa(This->mask));
            }
#endif
            reqaddr = reqaddr->next;
        }
    }

    return 0;
}

/* Return TRUE if addr represents a domain name */
PROXY_DECLARE(int) ap_proxy_is_domainname(struct dirconn_entry *This, apr_pool_t *p)
{
    char *addr = This->name;
    int i;

    /* Domain name must start with a '.' */
    if (addr[0] != '.') {
        return 0;
    }

    /* rfc1035 says DNS names must consist of "[-a-zA-Z0-9]" and '.' */
    for (i = 0; apr_isalnum(addr[i]) || addr[i] == '-' || addr[i] == '.'; ++i) {
        continue;
    }

#if 0
    if (addr[i] == ':') {
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                     "@@@@ handle optional port in proxy_is_domainname()");
    /* @@@@ handle optional port */
    }
#endif

    if (addr[i] != '\0') {
        return 0;
    }

    /* Strip trailing dots */
    for (i = strlen(addr) - 1; i > 0 && addr[i] == '.'; --i) {
        addr[i] = '\0';
    }

    This->matcher = proxy_match_domainname;
    return 1;
}

/* Return TRUE if host "host" is in domain "domain" */
static int proxy_match_domainname(struct dirconn_entry *This, request_rec *r)
{
    const char *host = proxy_get_host_of_request(r);
    int d_len = strlen(This->name), h_len;

    if (host == NULL) {      /* some error was logged already */
        return 0;
    }

    h_len = strlen(host);

    /* @@@ do this within the setup? */
    /* Ignore trailing dots in domain comparison: */
    while (d_len > 0 && This->name[d_len - 1] == '.') {
        --d_len;
    }
    while (h_len > 0 && host[h_len - 1] == '.') {
        --h_len;
    }
    return h_len > d_len
        && strncasecmp(&host[h_len - d_len], This->name, d_len) == 0;
}

/* Return TRUE if host represents a host name */
PROXY_DECLARE(int) ap_proxy_is_hostname(struct dirconn_entry *This, apr_pool_t *p)
{
    struct apr_sockaddr_t *addr;
    char *host = This->name;
    int i;

    /* Host names must not start with a '.' */
    if (host[0] == '.') {
        return 0;
    }
    /* rfc1035 says DNS names must consist of "[-a-zA-Z0-9]" and '.' */
    for (i = 0; apr_isalnum(host[i]) || host[i] == '-' || host[i] == '.'; ++i);

    if (host[i] != '\0' || apr_sockaddr_info_get(&addr, host, APR_UNSPEC, 0, 0, p) != APR_SUCCESS) {
        return 0;
    }

    This->hostaddr = addr;

    /* Strip trailing dots */
    for (i = strlen(host) - 1; i > 0 && host[i] == '.'; --i) {
        host[i] = '\0';
    }

    This->matcher = proxy_match_hostname;
    return 1;
}

/* Return TRUE if host "host" is equal to host2 "host2" */
static int proxy_match_hostname(struct dirconn_entry *This, request_rec *r)
{
    char *host = This->name;
    const char *host2 = proxy_get_host_of_request(r);
    int h2_len;
    int h1_len;

    if (host == NULL || host2 == NULL) {
        return 0; /* oops! */
    }

    h2_len = strlen(host2);
    h1_len = strlen(host);

#if 0
    struct apr_sockaddr_t *addr = *This->hostaddr;

    /* Try to deal with multiple IP addr's for a host */
    while (addr) {
        if (addr->ipaddr_ptr == ? ? ? ? ? ? ? ? ? ? ? ? ?)
            return 1;
        addr = addr->next;
    }
#endif

    /* Ignore trailing dots in host2 comparison: */
    while (h2_len > 0 && host2[h2_len - 1] == '.') {
        --h2_len;
    }
    while (h1_len > 0 && host[h1_len - 1] == '.') {
        --h1_len;
    }
    return h1_len == h2_len
        && strncasecmp(host, host2, h1_len) == 0;
}

/* Return TRUE if addr is to be matched as a word */
PROXY_DECLARE(int) ap_proxy_is_word(struct dirconn_entry *This, apr_pool_t *p)
{
    This->matcher = proxy_match_word;
    return 1;
}

/* Return TRUE if string "str2" occurs literally in "str1" */
static int proxy_match_word(struct dirconn_entry *This, request_rec *r)
{
    const char *host = proxy_get_host_of_request(r);
    return host != NULL && ap_strstr_c(host, This->name) != NULL;
}

/* Backwards-compatible interface. */
PROXY_DECLARE(int) ap_proxy_checkproxyblock(request_rec *r, proxy_server_conf *conf,
                             apr_sockaddr_t *uri_addr)
{
    return ap_proxy_checkproxyblock2(r, conf, uri_addr->hostname, uri_addr);
}

#define MAX_IP_STR_LEN (46)

PROXY_DECLARE(int) ap_proxy_checkproxyblock2(request_rec *r, proxy_server_conf *conf,
                                             const char *hostname, apr_sockaddr_t *addr)
{
    int j;

    /* XXX FIXME: conf->noproxies->elts is part of an opaque structure */
    for (j = 0; j < conf->noproxies->nelts; j++) {
        struct noproxy_entry *npent = (struct noproxy_entry *) conf->noproxies->elts;
        struct apr_sockaddr_t *conf_addr;

        ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                      "checking remote machine [%s] against [%s]",
                      hostname, npent[j].name);
        if (ap_strstr_c(hostname, npent[j].name) || npent[j].name[0] == '*') {
            ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(00916)
                          "connect to remote machine %s blocked: name %s "
                          "matched", hostname, npent[j].name);
            return HTTP_FORBIDDEN;
        }

        /* No IP address checks if no IP address was passed in,
         * i.e. the forward address proxy case, where this server does
         * not resolve the hostname.  */
        if (!addr)
            continue;

        for (conf_addr = npent[j].addr; conf_addr; conf_addr = conf_addr->next) {
            char caddr[MAX_IP_STR_LEN], uaddr[MAX_IP_STR_LEN];
            apr_sockaddr_t *uri_addr;

            if (apr_sockaddr_ip_getbuf(caddr, sizeof caddr, conf_addr))
                continue;

            for (uri_addr = addr; uri_addr; uri_addr = uri_addr->next) {
                if (apr_sockaddr_ip_getbuf(uaddr, sizeof uaddr, uri_addr))
                    continue;
                ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                              "ProxyBlock comparing %s and %s", caddr, uaddr);
                if (!strcmp(caddr, uaddr)) {
                    ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(00917)
                                  "connect to remote machine %s blocked: "
                                  "IP %s matched", hostname, caddr);
                    return HTTP_FORBIDDEN;
                }
            }
        }
    }

    return OK;
}

/* set up the minimal filter set */
PROXY_DECLARE(int) ap_proxy_pre_http_request(conn_rec *c, request_rec *r)
{
    ap_add_input_filter("HTTP_IN", NULL, r, c);
    return OK;
}

PROXY_DECLARE(const char *) ap_proxy_location_reverse_map(request_rec *r,
                              proxy_dir_conf *conf, const char *url)
{
    proxy_req_conf *rconf;
    struct proxy_alias *ent;
    int i, l1, l2;
    char *u;

    /*
     * XXX FIXME: Make sure this handled the ambiguous case of the :<PORT>
     * after the hostname
     * XXX FIXME: Ensure the /uri component is a case sensitive match
     */
    if (r->proxyreq != PROXYREQ_REVERSE) {
        return url;
    }

    l1 = strlen(url);
    if (conf->interpolate_env == 1) {
        rconf = ap_get_module_config(r->request_config, &proxy_module);
        ent = (struct proxy_alias *)rconf->raliases->elts;
    }
    else {
        ent = (struct proxy_alias *)conf->raliases->elts;
    }
    for (i = 0; i < conf->raliases->nelts; i++) {
        proxy_server_conf *sconf = (proxy_server_conf *)
            ap_get_module_config(r->server->module_config, &proxy_module);
        proxy_balancer *balancer;
        const char *real = ent[i].real;
        /*
         * First check if mapping against a balancer and see
         * if we have such a entity. If so, then we need to
         * find the particulars of the actual worker which may
         * or may not be the right one... basically, we need
         * to find which member actually handled this request.
         */
        if (ap_proxy_valid_balancer_name((char *)real, 0) &&
            (balancer = ap_proxy_get_balancer(r->pool, sconf, real, 1))) {
            int n, l3 = 0;
            proxy_worker **worker = (proxy_worker **)balancer->workers->elts;
            const char *urlpart = ap_strchr_c(real + sizeof(BALANCER_PREFIX) - 1, '/');
            if (urlpart) {
                if (!urlpart[1])
                    urlpart = NULL;
                else
                    l3 = strlen(urlpart);
            }
            /* The balancer comparison is a bit trickier.  Given the context
             *   BalancerMember balancer://alias http://example.com/foo
             *   ProxyPassReverse /bash balancer://alias/bar
             * translate url http://example.com/foo/bar/that to /bash/that
             */
            for (n = 0; n < balancer->workers->nelts; n++) {
                l2 = strlen((*worker)->s->name);
                if (urlpart) {
                    /* urlpart (l3) assuredly starts with its own '/' */
                    if ((*worker)->s->name[l2 - 1] == '/')
                        --l2;
                    if (l1 >= l2 + l3
                            && strncasecmp((*worker)->s->name, url, l2) == 0
                            && strncmp(urlpart, url + l2, l3) == 0) {
                        u = apr_pstrcat(r->pool, ent[i].fake, &url[l2 + l3],
                                        NULL);
                        return ap_is_url(u) ? u : ap_construct_url(r->pool, u, r);
                    }
                }
                else if (l1 >= l2 && strncasecmp((*worker)->s->name, url, l2) == 0) {
                    u = apr_pstrcat(r->pool, ent[i].fake, &url[l2], NULL);
                    return ap_is_url(u) ? u : ap_construct_url(r->pool, u, r);
                }
                worker++;
            }
        }
        else {
            const char *part = url;
            l2 = strlen(real);
            if (real[0] == '/') {
                part = ap_strstr_c(url, "://");
                if (part) {
                    part = ap_strchr_c(part+3, '/');
                    if (part) {
                        l1 = strlen(part);
                    }
                    else {
                        part = url;
                    }
                }
                else {
                    part = url;
                }
            }
            if (l1 >= l2 && strncasecmp(real, part, l2) == 0) {
                u = apr_pstrcat(r->pool, ent[i].fake, &part[l2], NULL);
                return ap_is_url(u) ? u : ap_construct_url(r->pool, u, r);
            }
        }
    }

    return url;
}

/*
 * Cookies are a bit trickier to match: we've got two substrings to worry
 * about, and we can't just find them with strstr 'cos of case.  Regexp
 * matching would be an easy fix, but for better consistency with all the
 * other matches we'll refrain and use apr_strmatch to find path=/domain=
 * and stick to plain strings for the config values.
 */
PROXY_DECLARE(const char *) ap_proxy_cookie_reverse_map(request_rec *r,
                              proxy_dir_conf *conf, const char *str)
{
    proxy_req_conf *rconf = ap_get_module_config(r->request_config,
                                                 &proxy_module);
    struct proxy_alias *ent;
    apr_size_t len = strlen(str);
    const char *newpath = NULL;
    const char *newdomain = NULL;
    const char *pathp;
    const char *domainp;
    const char *pathe = NULL;
    const char *domaine = NULL;
    apr_size_t l1, l2, poffs = 0, doffs = 0;
    int i;
    int ddiff = 0;
    int pdiff = 0;
    char *ret;

    if (r->proxyreq != PROXYREQ_REVERSE) {
        return str;
    }

   /*
    * Find the match and replacement, but save replacing until we've done
    * both path and domain so we know the new strlen
    */
    if ((pathp = apr_strmatch(ap_proxy_strmatch_path, str, len)) != NULL) {
        pathp += 5;
        poffs = pathp - str;
        pathe = ap_strchr_c(pathp, ';');
        l1 = pathe ? (pathe - pathp) : strlen(pathp);
        pathe = pathp + l1 ;
        if (conf->interpolate_env == 1) {
            ent = (struct proxy_alias *)rconf->cookie_paths->elts;
        }
        else {
            ent = (struct proxy_alias *)conf->cookie_paths->elts;
        }
        for (i = 0; i < conf->cookie_paths->nelts; i++) {
            l2 = strlen(ent[i].fake);
            if (l1 >= l2 && strncmp(ent[i].fake, pathp, l2) == 0) {
                newpath = ent[i].real;
                pdiff = strlen(newpath) - l1;
                break;
            }
        }
    }

    if ((domainp = apr_strmatch(ap_proxy_strmatch_domain, str, len)) != NULL) {
        domainp += 7;
        doffs = domainp - str;
        domaine = ap_strchr_c(domainp, ';');
        l1 = domaine ? (domaine - domainp) : strlen(domainp);
        domaine = domainp + l1;
        if (conf->interpolate_env == 1) {
            ent = (struct proxy_alias *)rconf->cookie_domains->elts;
        }
        else {
            ent = (struct proxy_alias *)conf->cookie_domains->elts;
        }
        for (i = 0; i < conf->cookie_domains->nelts; i++) {
            l2 = strlen(ent[i].fake);
            if (l1 >= l2 && strncasecmp(ent[i].fake, domainp, l2) == 0) {
                newdomain = ent[i].real;
                ddiff = strlen(newdomain) - l1;
                break;
            }
        }
    }

    if (newpath) {
        ret = apr_palloc(r->pool, len + pdiff + ddiff + 1);
        l1 = strlen(newpath);
        if (newdomain) {
            l2 = strlen(newdomain);
            if (doffs > poffs) {
                memcpy(ret, str, poffs);
                memcpy(ret + poffs, newpath, l1);
                memcpy(ret + poffs + l1, pathe, domainp - pathe);
                memcpy(ret + doffs + pdiff, newdomain, l2);
                strcpy(ret + doffs + pdiff + l2, domaine);
            }
            else {
                memcpy(ret, str, doffs) ;
                memcpy(ret + doffs, newdomain, l2);
                memcpy(ret + doffs + l2, domaine, pathp - domaine);
                memcpy(ret + poffs + ddiff, newpath, l1);
                strcpy(ret + poffs + ddiff + l1, pathe);
            }
        }
        else {
            memcpy(ret, str, poffs);
            memcpy(ret + poffs, newpath, l1);
            strcpy(ret + poffs + l1, pathe);
        }
    }
    else {
        if (newdomain) {
            ret = apr_palloc(r->pool, len + pdiff + ddiff + 1);
            l2 = strlen(newdomain);
            memcpy(ret, str, doffs);
            memcpy(ret + doffs, newdomain, l2);
            strcpy(ret + doffs+l2, domaine);
        }
        else {
            ret = (char *)str; /* no change */
        }
    }

    return ret;
}

/*
 * BALANCER related...
 */

/*
 * verifies that the balancer name conforms to standards.
 */
PROXY_DECLARE(int) ap_proxy_valid_balancer_name(char *name, int i)
{
    if (!i)
        i = sizeof(BALANCER_PREFIX)-1;
    return (!strncasecmp(name, BALANCER_PREFIX, i));
}


PROXY_DECLARE(proxy_balancer *) ap_proxy_get_balancer(apr_pool_t *p,
                                                      proxy_server_conf *conf,
                                                      const char *url,
                                                      int care)
{
    proxy_balancer *balancer;
    char *c, *uri = apr_pstrdup(p, url);
    int i;
    proxy_hashes hash;

    ap_str_tolower(uri);
    c = strchr(uri, ':');
    if (c == NULL || c[1] != '/' || c[2] != '/' || c[3] == '\0') {
        return NULL;
    }
    /* remove path from uri */
    if ((c = strchr(c + 3, '/'))) {
        *c = '\0';
    }
    hash.def = ap_proxy_hashfunc(uri, PROXY_HASHFUNC_DEFAULT);
    hash.fnv = ap_proxy_hashfunc(uri, PROXY_HASHFUNC_FNV);
    balancer = (proxy_balancer *)conf->balancers->elts;
    for (i = 0; i < conf->balancers->nelts; i++) {
        if (balancer->hash.def == hash.def && balancer->hash.fnv == hash.fnv) {
            if (!care || !balancer->s->inactive) {
                return balancer;
            }
        }
        balancer++;
    }
    return NULL;
}


PROXY_DECLARE(char *) ap_proxy_update_balancer(apr_pool_t *p,
                                                proxy_balancer *balancer,
                                                const char *url)
{
    apr_uri_t puri;
    if (apr_uri_parse(p, url, &puri) != APR_SUCCESS) {
        return apr_psprintf(p, "unable to parse: %s", url);
    }
    if (puri.path && PROXY_STRNCPY(balancer->s->vpath, puri.path) != APR_SUCCESS) {
        return apr_psprintf(p, "balancer %s front-end virtual-path (%s) too long",
                            balancer->s->name, puri.path);
    }
    if (puri.hostname && PROXY_STRNCPY(balancer->s->vhost, puri.hostname) != APR_SUCCESS) {
        return apr_psprintf(p, "balancer %s front-end vhost name (%s) too long",
                            balancer->s->name, puri.hostname);
    }
    return NULL;
}

PROXY_DECLARE(char *) ap_proxy_define_balancer(apr_pool_t *p,
                                               proxy_balancer **balancer,
                                               proxy_server_conf *conf,
                                               const char *url,
                                               const char *alias,
                                               int do_malloc)
{
    char nonce[APR_UUID_FORMATTED_LENGTH + 1];
    proxy_balancer_method *lbmethod;
    apr_uuid_t uuid;
    proxy_balancer_shared *bshared;
    char *c, *q, *uri = apr_pstrdup(p, url);
    const char *sname;

    /* We should never get here without a valid BALANCER_PREFIX... */

    c = strchr(uri, ':');
    if (c == NULL || c[1] != '/' || c[2] != '/' || c[3] == '\0')
        return "Bad syntax for a balancer name";
    /* remove path from uri */
    if ((q = strchr(c + 3, '/')))
        *q = '\0';

    ap_str_tolower(uri);
    *balancer = apr_array_push(conf->balancers);
    memset(*balancer, 0, sizeof(proxy_balancer));

    /*
     * NOTE: The default method is byrequests, which we assume
     * exists!
     */
    lbmethod = ap_lookup_provider(PROXY_LBMETHOD, "byrequests", "0");
    if (!lbmethod) {
        return "Can't find 'byrequests' lb method";
    }

    (*balancer)->workers = apr_array_make(p, 5, sizeof(proxy_worker *));
    (*balancer)->gmutex = NULL;
    (*balancer)->tmutex = NULL;
    (*balancer)->lbmethod = lbmethod;

    if (do_malloc)
        bshared = ap_malloc(sizeof(proxy_balancer_shared));
    else
        bshared = apr_palloc(p, sizeof(proxy_balancer_shared));

    memset(bshared, 0, sizeof(proxy_balancer_shared));

    bshared->was_malloced = (do_malloc != 0);
    PROXY_STRNCPY(bshared->lbpname, "byrequests");
    if (PROXY_STRNCPY(bshared->name, uri) != APR_SUCCESS) {
        return apr_psprintf(p, "balancer name (%s) too long", uri);
    }
    ap_pstr2_alnum(p, bshared->name + sizeof(BALANCER_PREFIX) - 1,
                   &sname);
    sname = apr_pstrcat(p, conf->id, "_", sname, NULL);
    if (PROXY_STRNCPY(bshared->sname, sname) != APR_SUCCESS) {
        return apr_psprintf(p, "balancer safe-name (%s) too long", sname);
    }
    bshared->hash.def = ap_proxy_hashfunc(bshared->name, PROXY_HASHFUNC_DEFAULT);
    bshared->hash.fnv = ap_proxy_hashfunc(bshared->name, PROXY_HASHFUNC_FNV);
    (*balancer)->hash = bshared->hash;

    bshared->forcerecovery = 1;

    /* Retrieve a UUID and store the nonce for the lifetime of
     * the process. */
    apr_uuid_get(&uuid);
    apr_uuid_format(nonce, &uuid);
    if (PROXY_STRNCPY(bshared->nonce, nonce) != APR_SUCCESS) {
        return apr_psprintf(p, "balancer nonce (%s) too long", nonce);
    }

    (*balancer)->s = bshared;
    (*balancer)->sconf = conf;

    return ap_proxy_update_balancer(p, *balancer, alias);
}

/*
 * Create an already defined balancer and free up memory.
 */
PROXY_DECLARE(apr_status_t) ap_proxy_share_balancer(proxy_balancer *balancer,
                                                    proxy_balancer_shared *shm,
                                                    int i)
{
    proxy_balancer_method *lbmethod;
    if (!shm || !balancer->s)
        return APR_EINVAL;

    memcpy(shm, balancer->s, sizeof(proxy_balancer_shared));
    if (balancer->s->was_malloced)
        free(balancer->s);
    balancer->s = shm;
    balancer->s->index = i;
    /* the below should always succeed */
    lbmethod = ap_lookup_provider(PROXY_LBMETHOD, balancer->s->lbpname, "0");
    if (lbmethod)
        balancer->lbmethod = lbmethod;
    return APR_SUCCESS;
}

PROXY_DECLARE(apr_status_t) ap_proxy_initialize_balancer(proxy_balancer *balancer, server_rec *s, apr_pool_t *p)
{
    apr_status_t rv = APR_SUCCESS;
    ap_slotmem_provider_t *storage = balancer->storage;
    apr_size_t size;
    unsigned int num;

    if (!storage) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s, APLOGNO(00918)
                     "no provider for %s", balancer->s->name);
        return APR_EGENERAL;
    }
    /*
     * for each balancer we need to init the global
     * mutex and then attach to the shared worker shm
     */
    if (!balancer->gmutex) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s, APLOGNO(00919)
                     "no mutex %s", balancer->s->name);
        return APR_EGENERAL;
    }

    /* Re-open the mutex for the child. */
    rv = apr_global_mutex_child_init(&(balancer->gmutex),
                                     apr_global_mutex_lockfile(balancer->gmutex),
                                     p);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, APLOGNO(00920)
                     "Failed to reopen mutex %s in child",
                     balancer->s->name);
        return rv;
    }

    /* now attach */
    storage->attach(&(balancer->wslot), balancer->s->sname, &size, &num, p);
    if (!balancer->wslot) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s, APLOGNO(00921) "slotmem_attach failed");
        return APR_EGENERAL;
    }
    if (balancer->lbmethod && balancer->lbmethod->reset)
        balancer->lbmethod->reset(balancer, s);

    if (balancer->tmutex == NULL) {
        rv = apr_thread_mutex_create(&(balancer->tmutex), APR_THREAD_MUTEX_DEFAULT, p);
        if (rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s, APLOGNO(00922)
                         "can not create balancer thread mutex");
            return rv;
        }
    }
    return APR_SUCCESS;
}

/*
 * CONNECTION related...
 */

static apr_status_t conn_pool_cleanup(void *theworker)
{
    proxy_worker *worker = (proxy_worker *)theworker;
    if (worker->cp->res) {
        worker->cp->pool = NULL;
    }
    return APR_SUCCESS;
}

static void init_conn_pool(apr_pool_t *p, proxy_worker *worker)
{
    apr_pool_t *pool;
    proxy_conn_pool *cp;

    /*
     * Create a connection pool's subpool.
     * This pool is used for connection recycling.
     * Once the worker is added it is never removed but
     * it can be disabled.
     */
    apr_pool_create(&pool, p);
    apr_pool_tag(pool, "proxy_worker_cp");
    /*
     * Alloc from the same pool as worker.
     * proxy_conn_pool is permanently attached to the worker.
     */
    cp = (proxy_conn_pool *)apr_pcalloc(p, sizeof(proxy_conn_pool));
    cp->pool = pool;
    worker->cp = cp;
}

static apr_status_t connection_cleanup(void *theconn)
{
    proxy_conn_rec *conn = (proxy_conn_rec *)theconn;
    proxy_worker *worker = conn->worker;

    /*
     * If the connection pool is NULL the worker
     * cleanup has been run. Just return.
     */
    if (!worker->cp) {
        return APR_SUCCESS;
    }

    if (conn->r) {
        apr_pool_destroy(conn->r->pool);
        conn->r = NULL;
    }

    /* Sanity check: Did we already return the pooled connection? */
    if (conn->inreslist) {
        ap_log_perror(APLOG_MARK, APLOG_ERR, 0, conn->pool, APLOGNO(00923)
                      "Pooled connection 0x%pp for worker %s has been"
                      " already returned to the connection pool.", conn,
                      worker->s->name);
        return APR_SUCCESS;
    }

    /* determine if the connection need to be closed */
    if (conn->close || !worker->s->is_address_reusable || worker->s->disablereuse) {
        apr_pool_t *p = conn->pool;
        apr_pool_clear(p);
        conn = apr_pcalloc(p, sizeof(proxy_conn_rec));
        conn->pool = p;
        conn->worker = worker;
        apr_pool_create(&(conn->scpool), p);
        apr_pool_tag(conn->scpool, "proxy_conn_scpool");
    }

    if (worker->s->hmax && worker->cp->res) {
        conn->inreslist = 1;
        apr_reslist_release(worker->cp->res, (void *)conn);
    }
    else
    {
        worker->cp->conn = conn;
    }

    /* Always return the SUCCESS */
    return APR_SUCCESS;
}

static void socket_cleanup(proxy_conn_rec *conn)
{
    conn->sock = NULL;
    conn->connection = NULL;
    apr_pool_clear(conn->scpool);
}

PROXY_DECLARE(apr_status_t) ap_proxy_ssl_connection_cleanup(proxy_conn_rec *conn,
                                                            request_rec *r)
{
    apr_bucket_brigade *bb;
    apr_status_t rv;

    /*
     * If we have an existing SSL connection it might be possible that the
     * server sent some SSL message we have not read so far (e.g. an SSL
     * shutdown message if the server closed the keepalive connection while
     * the connection was held unused in our pool).
     * So ensure that if present (=> APR_NONBLOCK_READ) it is read and
     * processed. We don't expect any data to be in the returned brigade.
     */
    if (conn->sock && conn->connection) {
        bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
        rv = ap_get_brigade(conn->connection->input_filters, bb,
                            AP_MODE_READBYTES, APR_NONBLOCK_READ,
                            HUGE_STRING_LEN);
        if ((rv != APR_SUCCESS) && !APR_STATUS_IS_EAGAIN(rv)) {
            socket_cleanup(conn);
        }
        if (!APR_BRIGADE_EMPTY(bb)) {
            apr_off_t len;

            rv = apr_brigade_length(bb, 0, &len);
            ap_log_rerror(APLOG_MARK, APLOG_TRACE3, rv, r,
                          "SSL cleanup brigade contained %"
                          APR_OFF_T_FMT " bytes of data.", len);
        }
        apr_brigade_destroy(bb);
    }
    return APR_SUCCESS;
}

/* reslist constructor */
static apr_status_t connection_constructor(void **resource, void *params,
                                           apr_pool_t *pool)
{
    apr_pool_t *ctx;
    apr_pool_t *scpool;
    proxy_conn_rec *conn;
    proxy_worker *worker = (proxy_worker *)params;

    /*
     * Create the subpool for each connection
     * This keeps the memory consumption constant
     * when disconnecting from backend.
     */
    apr_pool_create(&ctx, pool);
    apr_pool_tag(ctx, "proxy_conn_pool");
    /*
     * Create another subpool that manages the data for the
     * socket and the connection member of the proxy_conn_rec struct as we
     * destroy this data more frequently than other data in the proxy_conn_rec
     * struct like hostname and addr (at least in the case where we have
     * keepalive connections that timed out).
     */
    apr_pool_create(&scpool, ctx);
    apr_pool_tag(scpool, "proxy_conn_scpool");
    conn = apr_pcalloc(ctx, sizeof(proxy_conn_rec));

    conn->pool   = ctx;
    conn->scpool = scpool;
    conn->worker = worker;
    conn->inreslist = 1;
    *resource = conn;

    return APR_SUCCESS;
}

/* reslist destructor */
static apr_status_t connection_destructor(void *resource, void *params,
                                          apr_pool_t *pool)
{
    proxy_conn_rec *conn = (proxy_conn_rec *)resource;

    /* Destroy the pool only if not called from reslist_destroy */
    if (conn->worker->cp->pool) {
        apr_pool_destroy(conn->pool);
    }

    return APR_SUCCESS;
}

/*
 * WORKER related...
 */

PROXY_DECLARE(proxy_worker *) ap_proxy_get_worker(apr_pool_t *p,
                                                  proxy_balancer *balancer,
                                                  proxy_server_conf *conf,
                                                  const char *url)
{
    proxy_worker *worker;
    proxy_worker *max_worker = NULL;
    int max_match = 0;
    int url_length;
    int min_match;
    int worker_name_length;
    const char *c;
    char *url_copy;
    int i;

    c = ap_strchr_c(url, ':');
    if (c == NULL || c[1] != '/' || c[2] != '/' || c[3] == '\0') {
        return NULL;
    }

    url_length = strlen(url);
    url_copy = apr_pstrmemdup(p, url, url_length);

    /*
     * We need to find the start of the path and
     * therefore we know the length of the scheme://hostname/
     * part to we can force-lowercase everything up to
     * the start of the path.
     */
    c = ap_strchr_c(c+3, '/');
    if (c) {
        char *pathstart;
        pathstart = url_copy + (c - url);
        *pathstart = '\0';
        ap_str_tolower(url_copy);
        min_match = strlen(url_copy);
        *pathstart = '/';
    }
    else {
        ap_str_tolower(url_copy);
        min_match = strlen(url_copy);
    }
    /*
     * Do a "longest match" on the worker name to find the worker that
     * fits best to the URL, but keep in mind that we must have at least
     * a minimum matching of length min_match such that
     * scheme://hostname[:port] matches between worker and url.
     */

    if (balancer) {
        proxy_worker **workers = (proxy_worker **)balancer->workers->elts;
        for (i = 0; i < balancer->workers->nelts; i++, workers++) {
            worker = *workers;
            if ( ((worker_name_length = strlen(worker->s->name)) <= url_length)
                && (worker_name_length >= min_match)
                && (worker_name_length > max_match)
                && (strncmp(url_copy, worker->s->name, worker_name_length) == 0) ) {
                max_worker = worker;
                max_match = worker_name_length;
            }

        }
    } else {
        worker = (proxy_worker *)conf->workers->elts;
        for (i = 0; i < conf->workers->nelts; i++, worker++) {
            if ( ((worker_name_length = strlen(worker->s->name)) <= url_length)
                && (worker_name_length >= min_match)
                && (worker_name_length > max_match)
                && (strncmp(url_copy, worker->s->name, worker_name_length) == 0) ) {
                max_worker = worker;
                max_match = worker_name_length;
            }
        }
    }

    return max_worker;
}

/*
 * To create a worker from scratch first we define the
 * specifics of the worker; this is all local data.
 * We then allocate space for it if data needs to be
 * shared. This allows for dynamic addition during
 * config and runtime.
 */
PROXY_DECLARE(char *) ap_proxy_define_worker(apr_pool_t *p,
                                             proxy_worker **worker,
                                             proxy_balancer *balancer,
                                             proxy_server_conf *conf,
                                             const char *url,
                                             int do_malloc)
{
    int rv;
    apr_uri_t uri;
    proxy_worker_shared *wshared;
    char *ptr;

    rv = apr_uri_parse(p, url, &uri);

    if (rv != APR_SUCCESS) {
        return "Unable to parse URL";
    }
    if (!uri.hostname || !uri.scheme) {
        return "URL must be absolute!";
    }

    ap_str_tolower(uri.hostname);
    ap_str_tolower(uri.scheme);
    /*
     * Workers can be associated w/ balancers or on their
     * own; ie: the generic reverse-proxy or a worker
     * in a simple ProxyPass statement. eg:
     *
     *      ProxyPass / http://www.example.com
     *
     * in which case the worker goes in the conf slot.
     */
    if (balancer) {
        proxy_worker **runtime;
        /* recall that we get a ptr to the ptr here */
        runtime = apr_array_push(balancer->workers);
        *worker = *runtime = apr_palloc(p, sizeof(proxy_worker));   /* right to left baby */
        /* we've updated the list of workers associated with
         * this balancer *locally* */
        balancer->wupdated = apr_time_now();
    } else if (conf) {
        *worker = apr_array_push(conf->workers);
    } else {
        /* we need to allocate space here */
        *worker = apr_palloc(p, sizeof(proxy_worker));
    }

    memset(*worker, 0, sizeof(proxy_worker));
    /* right here we just want to tuck away the worker info.
     * if called during config, we don't have shm setup yet,
     * so just note the info for later. */
    if (do_malloc)
        wshared = ap_malloc(sizeof(proxy_worker_shared));  /* will be freed ap_proxy_share_worker */
    else
        wshared = apr_palloc(p, sizeof(proxy_worker_shared));

    memset(wshared, 0, sizeof(proxy_worker_shared));

    ptr = apr_uri_unparse(p, &uri, APR_URI_UNP_REVEALPASSWORD);
    if (PROXY_STRNCPY(wshared->name, ptr) != APR_SUCCESS) {
        return apr_psprintf(p, "worker name (%s) too long", ptr);
    }
    if (PROXY_STRNCPY(wshared->scheme, uri.scheme) != APR_SUCCESS) {
        return apr_psprintf(p, "worker scheme (%s) too long", uri.scheme);
    }
    if (PROXY_STRNCPY(wshared->hostname, uri.hostname) != APR_SUCCESS) {
        return apr_psprintf(p, "worker hostname (%s) too long", uri.hostname);
    }
    wshared->port = uri.port;
    wshared->flush_packets = flush_off;
    wshared->flush_wait = PROXY_FLUSH_WAIT;
    wshared->is_address_reusable = 1;
    wshared->lbfactor = 1;
    wshared->smax = -1;
    wshared->hash.def = ap_proxy_hashfunc(wshared->name, PROXY_HASHFUNC_DEFAULT);
    wshared->hash.fnv = ap_proxy_hashfunc(wshared->name, PROXY_HASHFUNC_FNV);
    wshared->was_malloced = (do_malloc != 0);

    (*worker)->hash = wshared->hash;
    (*worker)->context = NULL;
    (*worker)->cp = NULL;
    (*worker)->balancer = balancer;
    (*worker)->s = wshared;

    return NULL;
}

/*
 * Create an already defined worker and free up memory
 */
PROXY_DECLARE(apr_status_t) ap_proxy_share_worker(proxy_worker *worker, proxy_worker_shared *shm,
                                                  int i)
{
    if (!shm || !worker->s)
        return APR_EINVAL;

    memcpy(shm, worker->s, sizeof(proxy_worker_shared));
    if (worker->s->was_malloced)
        free(worker->s); /* was malloced in ap_proxy_define_worker */
    worker->s = shm;
    worker->s->index = i;
    return APR_SUCCESS;
}

PROXY_DECLARE(apr_status_t) ap_proxy_initialize_worker(proxy_worker *worker, server_rec *s, apr_pool_t *p)
{
    apr_status_t rv = APR_SUCCESS;
    int mpm_threads;

    if (worker->s->status & PROXY_WORKER_INITIALIZED) {
        /* The worker is already initialized */
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00924)
                     "worker %s shared already initialized", worker->s->name);
    }
    else {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00925)
                     "initializing worker %s shared", worker->s->name);
        /* Set default parameters */
        if (!worker->s->retry_set) {
            worker->s->retry = apr_time_from_sec(PROXY_WORKER_DEFAULT_RETRY);
        }
        /* By default address is reusable unless DisableReuse is set */
        if (worker->s->disablereuse) {
            worker->s->is_address_reusable = 0;
        }
        else {
            worker->s->is_address_reusable = 1;
        }

        ap_mpm_query(AP_MPMQ_MAX_THREADS, &mpm_threads);
        if (mpm_threads > 1) {
            /* Set hard max to no more then mpm_threads */
            if (worker->s->hmax == 0 || worker->s->hmax > mpm_threads) {
                worker->s->hmax = mpm_threads;
            }
            if (worker->s->smax == -1 || worker->s->smax > worker->s->hmax) {
                worker->s->smax = worker->s->hmax;
            }
            /* Set min to be lower then smax */
            if (worker->s->min > worker->s->smax) {
                worker->s->min = worker->s->smax;
            }
        }
        else {
            /* This will supress the apr_reslist creation */
            worker->s->min = worker->s->smax = worker->s->hmax = 0;
        }
    }

    /* What if local is init'ed and shm isn't?? Even possible? */
    if (worker->local_status & PROXY_WORKER_INITIALIZED) {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00926)
                     "worker %s local already initialized", worker->s->name);
    }
    else {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00927)
                     "initializing worker %s local", worker->s->name);
        /* Now init local worker data */
        if (worker->tmutex == NULL) {
            rv = apr_thread_mutex_create(&(worker->tmutex), APR_THREAD_MUTEX_DEFAULT, p);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00928)
                             "can not create worker thread mutex");
                return rv;
            }
        }
        if (worker->cp == NULL)
            init_conn_pool(p, worker);
        if (worker->cp == NULL) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00929)
                         "can not create connection pool");
            return APR_EGENERAL;
        }

        if (worker->s->hmax) {
            rv = apr_reslist_create(&(worker->cp->res),
                                    worker->s->min, worker->s->smax,
                                    worker->s->hmax, worker->s->ttl,
                                    connection_constructor, connection_destructor,
                                    worker, worker->cp->pool);

            apr_pool_cleanup_register(worker->cp->pool, (void *)worker,
                                      conn_pool_cleanup,
                                      apr_pool_cleanup_null);

            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00930)
                "initialized pool in child %" APR_PID_T_FMT " for (%s) min=%d max=%d smax=%d",
                 getpid(), worker->s->hostname, worker->s->min,
                 worker->s->hmax, worker->s->smax);

            /* Set the acquire timeout */
            if (rv == APR_SUCCESS && worker->s->acquire_set) {
                apr_reslist_timeout_set(worker->cp->res, worker->s->acquire);
            }

        }
        else {
            void *conn;

            rv = connection_constructor(&conn, worker, worker->cp->pool);
            worker->cp->conn = conn;

            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00931)
                 "initialized single connection worker in child %" APR_PID_T_FMT " for (%s)",
                 getpid(), worker->s->hostname);
        }
    }
    if (rv == APR_SUCCESS) {
        worker->s->status |= (PROXY_WORKER_INITIALIZED);
        worker->local_status |= (PROXY_WORKER_INITIALIZED);
    }
    return rv;
}

static int ap_proxy_retry_worker(const char *proxy_function, proxy_worker *worker,
        server_rec *s)
{
    if (worker->s->status & PROXY_WORKER_IN_ERROR) {
        if (apr_time_now() > worker->s->error_time + worker->s->retry) {
            ++worker->s->retries;
            worker->s->status &= ~PROXY_WORKER_IN_ERROR;
            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00932)
                         "%s: worker for (%s) has been marked for retry",
                         proxy_function, worker->s->hostname);
            return OK;
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00933)
                         "%s: too soon to retry worker for (%s)",
                         proxy_function, worker->s->hostname);
            return DECLINED;
        }
    }
    else {
        return OK;
    }
}

PROXY_DECLARE(int) ap_proxy_pre_request(proxy_worker **worker,
                                        proxy_balancer **balancer,
                                        request_rec *r,
                                        proxy_server_conf *conf, char **url)
{
    int access_status;

    access_status = proxy_run_pre_request(worker, balancer, r, conf, url);
    if (access_status == DECLINED && *balancer == NULL) {
        *worker = ap_proxy_get_worker(r->pool, NULL, conf, *url);
        if (*worker) {
            ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                          "%s: found worker %s for %s",
                          (*worker)->s->scheme, (*worker)->s->name, *url);

            *balancer = NULL;
            access_status = OK;
        }
        else if (r->proxyreq == PROXYREQ_PROXY) {
            if (conf->forward) {
                ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                              "*: found forward proxy worker for %s", *url);
                *balancer = NULL;
                *worker = conf->forward;
                access_status = OK;
                /*
                 * The forward worker does not keep connections alive, so
                 * ensure that mod_proxy_http does the correct thing
                 * regarding the Connection header in the request.
                 */
                apr_table_setn(r->subprocess_env, "proxy-nokeepalive", "1");
            }
        }
        else if (r->proxyreq == PROXYREQ_REVERSE) {
            if (conf->reverse) {
                ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                              "*: found reverse proxy worker for %s", *url);
                *balancer = NULL;
                *worker = conf->reverse;
                access_status = OK;
                /*
                 * The reverse worker does not keep connections alive, so
                 * ensure that mod_proxy_http does the correct thing
                 * regarding the Connection header in the request.
                 */
                apr_table_setn(r->subprocess_env, "proxy-nokeepalive", "1");
            }
        }
    }
    else if (access_status == DECLINED && *balancer != NULL) {
        /* All the workers are busy */
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00934)
                      "all workers are busy.  Unable to serve %s", *url);
        access_status = HTTP_SERVICE_UNAVAILABLE;
    }
    return access_status;
}

PROXY_DECLARE(int) ap_proxy_post_request(proxy_worker *worker,
                                         proxy_balancer *balancer,
                                         request_rec *r,
                                         proxy_server_conf *conf)
{
    int access_status = OK;
    if (balancer) {
        access_status = proxy_run_post_request(worker, balancer, r, conf);
        if (access_status == DECLINED) {
            access_status = OK; /* no post_request handler available */
            /* TODO: recycle direct worker */
        }
    }

    return access_status;
}

/* DEPRECATED */
PROXY_DECLARE(int) ap_proxy_connect_to_backend(apr_socket_t **newsock,
                                               const char *proxy_function,
                                               apr_sockaddr_t *backend_addr,
                                               const char *backend_name,
                                               proxy_server_conf *conf,
                                               request_rec *r)
{
    apr_status_t rv;
    int connected = 0;
    int loglevel;

    while (backend_addr && !connected) {
        if ((rv = apr_socket_create(newsock, backend_addr->family,
                                    SOCK_STREAM, 0, r->pool)) != APR_SUCCESS) {
            loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
            ap_log_rerror(APLOG_MARK, loglevel, rv, r, APLOGNO(00935)
                          "%s: error creating fam %d socket for target %s",
                          proxy_function, backend_addr->family, backend_name);
            /*
             * this could be an IPv6 address from the DNS but the
             * local machine won't give us an IPv6 socket; hopefully the
             * DNS returned an additional address to try
             */
            backend_addr = backend_addr->next;
            continue;
        }

        if (conf->recv_buffer_size > 0 &&
            (rv = apr_socket_opt_set(*newsock, APR_SO_RCVBUF,
                                     conf->recv_buffer_size))) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(00936)
                          "apr_socket_opt_set(SO_RCVBUF): Failed to set "
                          "ProxyReceiveBufferSize, using default");
        }

        rv = apr_socket_opt_set(*newsock, APR_TCP_NODELAY, 1);
        if (rv != APR_SUCCESS && rv != APR_ENOTIMPL) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(00937)
                          "apr_socket_opt_set(APR_TCP_NODELAY): "
                          "Failed to set");
        }

        /* Set a timeout on the socket */
        if (conf->timeout_set) {
            apr_socket_timeout_set(*newsock, conf->timeout);
        }
        else {
            apr_socket_timeout_set(*newsock, r->server->timeout);
        }

        ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                      "%s: fam %d socket created to connect to %s",
                      proxy_function, backend_addr->family, backend_name);

        if (conf->source_address) {
            rv = apr_socket_bind(*newsock, conf->source_address);
            if (rv != APR_SUCCESS) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(00938)
                              "%s: failed to bind socket to local address",
                              proxy_function);
            }
        }

        /* make the connection out of the socket */
        rv = apr_socket_connect(*newsock, backend_addr);

        /* if an error occurred, loop round and try again */
        if (rv != APR_SUCCESS) {
            apr_socket_close(*newsock);
            loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
            ap_log_rerror(APLOG_MARK, loglevel, rv, r, APLOGNO(00939)
                          "%s: attempt to connect to %pI (%s) failed",
                          proxy_function, backend_addr, backend_name);
            backend_addr = backend_addr->next;
            continue;
        }
        connected = 1;
    }
    return connected ? 0 : 1;
}

PROXY_DECLARE(int) ap_proxy_acquire_connection(const char *proxy_function,
                                               proxy_conn_rec **conn,
                                               proxy_worker *worker,
                                               server_rec *s)
{
    apr_status_t rv;

    if (!PROXY_WORKER_IS_USABLE(worker)) {
        /* Retry the worker */
        ap_proxy_retry_worker(proxy_function, worker, s);

        if (!PROXY_WORKER_IS_USABLE(worker)) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00940)
                         "%s: disabled connection for (%s)",
                         proxy_function, worker->s->hostname);
            return HTTP_SERVICE_UNAVAILABLE;
        }
    }

    if (worker->s->hmax && worker->cp->res) {
        rv = apr_reslist_acquire(worker->cp->res, (void **)conn);
    }
    else {
        /* create the new connection if the previous was destroyed */
        if (!worker->cp->conn) {
            connection_constructor((void **)conn, worker, worker->cp->pool);
        }
        else {
            *conn = worker->cp->conn;
            worker->cp->conn = NULL;
        }
        rv = APR_SUCCESS;
    }

    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00941)
                     "%s: failed to acquire connection for (%s)",
                     proxy_function, worker->s->hostname);
        return HTTP_SERVICE_UNAVAILABLE;
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00942)
                 "%s: has acquired connection for (%s)",
                 proxy_function, worker->s->hostname);

    (*conn)->worker = worker;
    (*conn)->close  = 0;
    (*conn)->inreslist = 0;

    return OK;
}

PROXY_DECLARE(int) ap_proxy_release_connection(const char *proxy_function,
                                               proxy_conn_rec *conn,
                                               server_rec *s)
{
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00943)
                "%s: has released connection for (%s)",
                proxy_function, conn->worker->s->hostname);
    connection_cleanup(conn);

    return OK;
}

PROXY_DECLARE(int)
ap_proxy_determine_connection(apr_pool_t *p, request_rec *r,
                              proxy_server_conf *conf,
                              proxy_worker *worker,
                              proxy_conn_rec *conn,
                              apr_uri_t *uri,
                              char **url,
                              const char *proxyname,
                              apr_port_t proxyport,
                              char *server_portstr,
                              int server_portstr_size)
{
    int server_port;
    apr_status_t err = APR_SUCCESS;
    apr_status_t uerr = APR_SUCCESS;

    /*
     * Break up the URL to determine the host to connect to
     */

    /* we break the URL into host, port, uri */
    if (APR_SUCCESS != apr_uri_parse(p, *url, uri)) {
        return ap_proxyerror(r, HTTP_BAD_REQUEST,
                             apr_pstrcat(p,"URI cannot be parsed: ", *url,
                                         NULL));
    }
    if (!uri->port) {
        uri->port = apr_uri_port_of_scheme(uri->scheme);
    }

    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(00944)
                 "connecting %s to %s:%d", *url, uri->hostname, uri->port);

    /*
     * allocate these out of the specified connection pool
     * The scheme handler decides if this is permanent or
     * short living pool.
     */
    /* are we connecting directly, or via a proxy? */
    if (!proxyname) {
        *url = apr_pstrcat(p, uri->path, uri->query ? "?" : "",
                           uri->query ? uri->query : "",
                           uri->fragment ? "#" : "",
                           uri->fragment ? uri->fragment : "", NULL);
    }
    /*
     * Figure out if our passed in proxy_conn_rec has a usable
     * address cached.
     *
     * TODO: Handle this much better... 
     *
     * XXX: If generic workers are ever address-reusable, we need 
     *      to check host and port on the conn and be careful about
     *      spilling the cached addr from the worker.
     */
    if (!conn->hostname || !worker->s->is_address_reusable ||
        worker->s->disablereuse) {
        if (proxyname) {
            conn->hostname = apr_pstrdup(conn->pool, proxyname);
            conn->port = proxyport;
            /*
             * If we have a forward proxy and the protocol is HTTPS,
             * then we need to prepend a HTTP CONNECT request before
             * sending our actual HTTPS requests.
             * Save our real backend data for using it later during HTTP CONNECT.
             */
            if (conn->is_ssl) {
                const char *proxy_auth;

                forward_info *forward = apr_pcalloc(conn->pool, sizeof(forward_info));
                conn->forward = forward;
                forward->use_http_connect = 1;
                forward->target_host = apr_pstrdup(conn->pool, uri->hostname);
                forward->target_port = uri->port;
                /* Do we want to pass Proxy-Authorization along?
                 * If we haven't used it, then YES
                 * If we have used it then MAYBE: RFC2616 says we MAY propagate it.
                 * So let's make it configurable by env.
                 * The logic here is the same used in mod_proxy_http.
                 */
                proxy_auth = apr_table_get(r->headers_in, "Proxy-Authorization");
                if (proxy_auth != NULL &&
                    proxy_auth[0] != '\0' &&
                    r->user == NULL && /* we haven't yet authenticated */
                    apr_table_get(r->subprocess_env, "Proxy-Chain-Auth")) {
                    forward->proxy_auth = apr_pstrdup(conn->pool, proxy_auth);
                }
            }
        }
        else {
            conn->hostname = apr_pstrdup(conn->pool, uri->hostname);
            conn->port = uri->port;
        }
        socket_cleanup(conn);
        err = apr_sockaddr_info_get(&(conn->addr),
                                    conn->hostname, APR_UNSPEC,
                                    conn->port, 0,
                                    conn->pool);
    }
    else if (!worker->cp->addr) {
        if ((err = PROXY_THREAD_LOCK(worker)) != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, err, r, APLOGNO(00945) "lock");
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        /*
         * Worker can have the single constant backend adress.
         * The single DNS lookup is used once per worker.
         * If dynamic change is needed then set the addr to NULL
         * inside dynamic config to force the lookup.
         */
        err = apr_sockaddr_info_get(&(worker->cp->addr),
                                    conn->hostname, APR_UNSPEC,
                                    conn->port, 0,
                                    worker->cp->pool);
        conn->addr = worker->cp->addr;
        if ((uerr = PROXY_THREAD_UNLOCK(worker)) != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, uerr, r, APLOGNO(00946) "unlock");
        }
    }
    else {
        conn->addr = worker->cp->addr;
    }
    /* Close a possible existing socket if we are told to do so */
    if (conn->close) {
        socket_cleanup(conn);
        conn->close = 0;
    }

    if (err != APR_SUCCESS) {
        return ap_proxyerror(r, HTTP_BAD_GATEWAY,
                             apr_pstrcat(p, "DNS lookup failure for: ",
                                         conn->hostname, NULL));
    }

    /* Get the server port for the Via headers */
    {
        server_port = ap_get_server_port(r);
        if (ap_is_default_port(server_port, r)) {
            strcpy(server_portstr,"");
        }
        else {
            apr_snprintf(server_portstr, server_portstr_size, ":%d",
                         server_port);
        }
    }
    /* check if ProxyBlock directive on this host */
    if (OK != ap_proxy_checkproxyblock2(r, conf, uri->hostname, 
                                       proxyname ? NULL : conn->addr)) {
        return ap_proxyerror(r, HTTP_FORBIDDEN,
                             "Connect to remote machine blocked");
    }
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(00947)
                 "connected %s to %s:%d", *url, conn->hostname, conn->port);
    return OK;
}

#define USE_ALTERNATE_IS_CONNECTED 1

#if !defined(APR_MSG_PEEK) && defined(MSG_PEEK)
#define APR_MSG_PEEK MSG_PEEK
#endif

#if USE_ALTERNATE_IS_CONNECTED && defined(APR_MSG_PEEK)
static int is_socket_connected(apr_socket_t *socket)
{
    apr_pollfd_t pfds[1];
    apr_status_t status;
    apr_int32_t  nfds;

    pfds[0].reqevents = APR_POLLIN;
    pfds[0].desc_type = APR_POLL_SOCKET;
    pfds[0].desc.s = socket;

    do {
        status = apr_poll(&pfds[0], 1, &nfds, 0);
    } while (APR_STATUS_IS_EINTR(status));

    if (status == APR_SUCCESS && nfds == 1 &&
        pfds[0].rtnevents == APR_POLLIN) {
        apr_sockaddr_t unused;
        apr_size_t len = 1;
        char buf[1];
        /* The socket might be closed in which case
         * the poll will return POLLIN.
         * If there is no data available the socket
         * is closed.
         */
        status = apr_socket_recvfrom(&unused, socket, APR_MSG_PEEK,
                                     &buf[0], &len);
        if (status == APR_SUCCESS && len)
            return 1;
        else
            return 0;
    }
    else if (APR_STATUS_IS_EAGAIN(status) || APR_STATUS_IS_TIMEUP(status)) {
        return 1;
    }
    return 0;

}
#else
static int is_socket_connected(apr_socket_t *sock)

{
    apr_size_t buffer_len = 1;
    char test_buffer[1];
    apr_status_t socket_status;
    apr_interval_time_t current_timeout;

    /* save timeout */
    apr_socket_timeout_get(sock, &current_timeout);
    /* set no timeout */
    apr_socket_timeout_set(sock, 0);
    socket_status = apr_socket_recv(sock, test_buffer, &buffer_len);
    /* put back old timeout */
    apr_socket_timeout_set(sock, current_timeout);
    if (APR_STATUS_IS_EOF(socket_status)
        || APR_STATUS_IS_ECONNRESET(socket_status)) {
        return 0;
    }
    else {
        return 1;
    }
}
#endif /* USE_ALTERNATE_IS_CONNECTED */


/*
 * Send a HTTP CONNECT request to a forward proxy.
 * The proxy is given by "backend", the target server
 * is contained in the "forward" member of "backend".
 */
static apr_status_t send_http_connect(proxy_conn_rec *backend,
                                      server_rec *s)
{
    int status;
    apr_size_t nbytes;
    apr_size_t left;
    int complete = 0;
    char buffer[HUGE_STRING_LEN];
    char drain_buffer[HUGE_STRING_LEN];
    forward_info *forward = (forward_info *)backend->forward;
    int len = 0;

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00948)
                 "CONNECT: sending the CONNECT request for %s:%d "
                 "to the remote proxy %pI (%s)",
                 forward->target_host, forward->target_port,
                 backend->addr, backend->hostname);
    /* Create the CONNECT request */
    nbytes = apr_snprintf(buffer, sizeof(buffer),
                          "CONNECT %s:%d HTTP/1.0" CRLF,
                          forward->target_host, forward->target_port);
    /* Add proxy authorization from the initial request if necessary */
    if (forward->proxy_auth != NULL) {
        nbytes += apr_snprintf(buffer + nbytes, sizeof(buffer) - nbytes,
                               "Proxy-Authorization: %s" CRLF,
                               forward->proxy_auth);
    }
    /* Set a reasonable agent and send everything */
    nbytes += apr_snprintf(buffer + nbytes, sizeof(buffer) - nbytes,
                           "Proxy-agent: %s" CRLF CRLF,
                           ap_get_server_banner());
    apr_socket_send(backend->sock, buffer, &nbytes);

    /* Receive the whole CONNECT response */
    left = sizeof(buffer) - 1;
    /* Read until we find the end of the headers or run out of buffer */
    do {
        nbytes = left;
        status = apr_socket_recv(backend->sock, buffer + len, &nbytes);
        len += nbytes;
        left -= nbytes;
        buffer[len] = '\0';
        if (strstr(buffer + len - nbytes, "\r\n\r\n") != NULL) {
            complete = 1;
            break;
        }
    } while (status == APR_SUCCESS && left > 0);
    /* Drain what's left */
    if (!complete) {
        nbytes = sizeof(drain_buffer) - 1;
        while (status == APR_SUCCESS && nbytes) {
            status = apr_socket_recv(backend->sock, drain_buffer, &nbytes);
            buffer[nbytes] = '\0';
            nbytes = sizeof(drain_buffer) - 1;
            if (strstr(drain_buffer, "\r\n\r\n") != NULL) {
                break;
            }
        }
    }

    /* Check for HTTP_OK response status */
    if (status == APR_SUCCESS) {
        int major, minor;
        /* Only scan for three character status code */
        char code_str[4];

        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00949)
                     "send_http_connect: response from the forward proxy: %s",
                     buffer);

        /* Extract the returned code */
        if (sscanf(buffer, "HTTP/%u.%u %3s", &major, &minor, code_str) == 3) {
            status = atoi(code_str);
            if (status == HTTP_OK) {
                status = APR_SUCCESS;
            }
            else {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00950)
                             "send_http_connect: the forward proxy returned code is '%s'",
                             code_str);
            status = APR_INCOMPLETE;
            }
        }
    }

    return(status);
}


PROXY_DECLARE(int) ap_proxy_connect_backend(const char *proxy_function,
                                            proxy_conn_rec *conn,
                                            proxy_worker *worker,
                                            server_rec *s)
{
    apr_status_t rv;
    int connected = 0;
    int loglevel;
    apr_sockaddr_t *backend_addr = conn->addr;
    /* the local address to use for the outgoing connection */
    apr_sockaddr_t *local_addr;
    apr_socket_t *newsock;
    void *sconf = s->module_config;
    proxy_server_conf *conf =
        (proxy_server_conf *) ap_get_module_config(sconf, &proxy_module);

    if (conn->sock) {
        if (!(connected = is_socket_connected(conn->sock))) {
            socket_cleanup(conn);
            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00951)
                         "%s: backend socket is disconnected.",
                         proxy_function);
        }
    }
    while (backend_addr && !connected) {
        if ((rv = apr_socket_create(&newsock, backend_addr->family,
                                SOCK_STREAM, APR_PROTO_TCP,
                                conn->scpool)) != APR_SUCCESS) {
            loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
            ap_log_error(APLOG_MARK, loglevel, rv, s, APLOGNO(00952)
                         "%s: error creating fam %d socket for target %s",
                         proxy_function,
                         backend_addr->family,
                         worker->s->hostname);
            /*
             * this could be an IPv6 address from the DNS but the
             * local machine won't give us an IPv6 socket; hopefully the
             * DNS returned an additional address to try
             */
            backend_addr = backend_addr->next;
            continue;
        }
        conn->connection = NULL;

        if (worker->s->recv_buffer_size > 0 &&
            (rv = apr_socket_opt_set(newsock, APR_SO_RCVBUF,
                                     worker->s->recv_buffer_size))) {
            ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00953)
                         "apr_socket_opt_set(SO_RCVBUF): Failed to set "
                         "ProxyReceiveBufferSize, using default");
        }

        rv = apr_socket_opt_set(newsock, APR_TCP_NODELAY, 1);
        if (rv != APR_SUCCESS && rv != APR_ENOTIMPL) {
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00954)
                          "apr_socket_opt_set(APR_TCP_NODELAY): "
                          "Failed to set");
        }

        /* Set a timeout for connecting to the backend on the socket */
        if (worker->s->conn_timeout_set) {
            apr_socket_timeout_set(newsock, worker->s->conn_timeout);
        }
        else if (worker->s->timeout_set) {
            apr_socket_timeout_set(newsock, worker->s->timeout);
        }
        else if (conf->timeout_set) {
            apr_socket_timeout_set(newsock, conf->timeout);
        }
        else {
             apr_socket_timeout_set(newsock, s->timeout);
        }
        /* Set a keepalive option */
        if (worker->s->keepalive) {
            if ((rv = apr_socket_opt_set(newsock,
                            APR_SO_KEEPALIVE, 1)) != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00955)
                             "apr_socket_opt_set(SO_KEEPALIVE): Failed to set"
                             " Keepalive");
            }
        }
        ap_log_error(APLOG_MARK, APLOG_TRACE2, 0, s,
                     "%s: fam %d socket created to connect to %s",
                     proxy_function, backend_addr->family, worker->s->hostname);

        if (conf->source_address_set) {
            local_addr = apr_pmemdup(conn->pool, conf->source_address,
                                     sizeof(apr_sockaddr_t));
            local_addr->pool = conn->pool;
            rv = apr_socket_bind(newsock, local_addr);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00956)
                    "%s: failed to bind socket to local address",
                    proxy_function);
            }
        }

        /* make the connection out of the socket */
        rv = apr_socket_connect(newsock, backend_addr);

        /* if an error occurred, loop round and try again */
        if (rv != APR_SUCCESS) {
            apr_socket_close(newsock);
            loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
            ap_log_error(APLOG_MARK, loglevel, rv, s, APLOGNO(00957)
                         "%s: attempt to connect to %pI (%s) failed",
                         proxy_function,
                         backend_addr,
                         worker->s->hostname);
            backend_addr = backend_addr->next;
            continue;
        }

        /* Set a timeout on the socket */
        if (worker->s->timeout_set) {
            apr_socket_timeout_set(newsock, worker->s->timeout);
        }
        else if (conf->timeout_set) {
            apr_socket_timeout_set(newsock, conf->timeout);
        }
        else {
             apr_socket_timeout_set(newsock, s->timeout);
        }

        conn->sock = newsock;

        if (conn->forward) {
            forward_info *forward = (forward_info *)conn->forward;
            /*
             * For HTTP CONNECT we need to prepend CONNECT request before
             * sending our actual HTTPS requests.
             */
            if (forward->use_http_connect) {
                rv = send_http_connect(conn, s);
                /* If an error occurred, loop round and try again */
                if (rv != APR_SUCCESS) {
                    conn->sock = NULL;
                    apr_socket_close(newsock);
                    loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
                    ap_log_error(APLOG_MARK, loglevel, rv, s, APLOGNO(00958)
                                 "%s: attempt to connect to %s:%d "
                                 "via http CONNECT through %pI (%s) failed",
                                 proxy_function,
                                 forward->target_host, forward->target_port,
                                 backend_addr, worker->s->hostname);
                    backend_addr = backend_addr->next;
                    continue;
                }
            }
        }

        connected    = 1;
    }
    /*
     * Put the entire worker to error state if
     * the PROXY_WORKER_IGNORE_ERRORS flag is not set.
     * Altrough some connections may be alive
     * no further connections to the worker could be made
     */
    if (!connected && PROXY_WORKER_IS_USABLE(worker) &&
        !(worker->s->status & PROXY_WORKER_IGNORE_ERRORS)) {
        worker->s->error_time = apr_time_now();
        worker->s->status |= PROXY_WORKER_IN_ERROR;
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00959)
            "ap_proxy_connect_backend disabling worker for (%s) for %"
            APR_TIME_T_FMT "s",
            worker->s->hostname, apr_time_sec(worker->s->retry));
    }
    else {
        if (worker->s->retries) {
            /*
             * A worker came back. So here is where we need to
             * either reset all params to initial conditions or
             * apply some sort of aging
             */
        }
        worker->s->error_time = 0;
        worker->s->retries = 0;
    }
    return connected ? OK : DECLINED;
}

PROXY_DECLARE(int) ap_proxy_connection_create(const char *proxy_function,
                                              proxy_conn_rec *conn,
                                              conn_rec *c,
                                              server_rec *s)
{
    apr_sockaddr_t *backend_addr = conn->addr;
    int rc;
    apr_interval_time_t current_timeout;
    apr_bucket_alloc_t *bucket_alloc;

    if (conn->connection) {
        return OK;
    }

    bucket_alloc = apr_bucket_alloc_create(conn->scpool);
    /*
     * The socket is now open, create a new backend server connection
     */
    conn->connection = ap_run_create_connection(conn->scpool, s, conn->sock,
                                                0, NULL,
                                                bucket_alloc);

    if (!conn->connection) {
        /*
         * the peer reset the connection already; ap_run_create_connection()
         * closed the socket
         */
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
                     s, APLOGNO(00960) "%s: an error occurred creating a "
                     "new connection to %pI (%s)", proxy_function,
                     backend_addr, conn->hostname);
        /* XXX: Will be closed when proxy_conn is closed */
        socket_cleanup(conn);
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    /* For ssl connection to backend */
    if (conn->is_ssl) {
        if (!ap_proxy_ssl_enable(conn->connection)) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0,
                         s, APLOGNO(00961) "%s: failed to enable ssl support "
                         "for %pI (%s)", proxy_function,
                         backend_addr, conn->hostname);
            return HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    else {
        /* TODO: See if this will break FTP */
        ap_proxy_ssl_disable(conn->connection);
    }

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00962)
                 "%s: connection complete to %pI (%s)",
                 proxy_function, backend_addr, conn->hostname);

    /*
     * save the timeout of the socket because core_pre_connection
     * will set it to base_server->timeout
     * (core TimeOut directive).
     */
    apr_socket_timeout_get(conn->sock, &current_timeout);
    /* set up the connection filters */
    rc = ap_run_pre_connection(conn->connection, conn->sock);
    if (rc != OK && rc != DONE) {
        conn->connection->aborted = 1;
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00963)
                     "%s: pre_connection setup failed (%d)",
                     proxy_function, rc);
        return rc;
    }
    apr_socket_timeout_set(conn->sock, current_timeout);

    return OK;
}

int ap_proxy_lb_workers(void)
{
    /*
     * Since we can't resize the scoreboard when reconfiguring, we
     * have to impose a limit on the number of workers, we are
     * able to reconfigure to.
     */
    if (!lb_workers_limit)
        lb_workers_limit = proxy_lb_workers + PROXY_DYNAMIC_BALANCER_LIMIT;
    return lb_workers_limit;
}

PROXY_DECLARE(void) ap_proxy_backend_broke(request_rec *r,
                                           apr_bucket_brigade *brigade)
{
    apr_bucket *e;
    conn_rec *c = r->connection;

    r->no_cache = 1;
    /*
     * If this is a subrequest, then prevent also caching of the main
     * request.
     */
    if (r->main)
        r->main->no_cache = 1;
    e = ap_bucket_error_create(HTTP_BAD_GATEWAY, NULL, c->pool,
                               c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(brigade, e);
    e = apr_bucket_eos_create(c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(brigade, e);
}

/*
 * Provide a string hashing function for the proxy.
 * We offer 2 methods: one is the APR model but we
 * also provide our own, based on either FNV or SDBM.
 * The reason is in case we want to use both to ensure no
 * collisions.
 */
PROXY_DECLARE(unsigned int)
ap_proxy_hashfunc(const char *str, proxy_hash_t method)
{
    if (method == PROXY_HASHFUNC_APR) {
        apr_ssize_t slen = strlen(str);
        return apr_hashfunc_default(str, &slen);
    }
    else if (method == PROXY_HASHFUNC_FNV) {
        /* FNV model */
        unsigned int hash;
        const unsigned int fnv_prime = 0x811C9DC5;
        for (hash = 0; *str; str++) {
            hash *= fnv_prime;
            hash ^= (*str);
        }
        return hash;
    }
    else { /* method == PROXY_HASHFUNC_DEFAULT */
        /* SDBM model */
        unsigned int hash;
        for (hash = 0; *str; str++) {
            hash = (*str) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
}

PROXY_DECLARE(apr_status_t) ap_proxy_set_wstatus(char c, int set, proxy_worker *w)
{
    unsigned int *status = &w->s->status;
    char flag = toupper(c);
    struct wstat *pwt = wstat_tbl;
    while (pwt->bit) {
        if (flag == pwt->flag) {
            if (set)
                *status |= pwt->bit;
            else
                *status &= ~(pwt->bit);
            return APR_SUCCESS;
        }
        pwt++;
    }
    return APR_EINVAL;
}

PROXY_DECLARE(char *) ap_proxy_parse_wstatus(apr_pool_t *p, proxy_worker *w)
{
    char *ret = "";
    unsigned int status = w->s->status;
    struct wstat *pwt = wstat_tbl;
    while (pwt->bit) {
        if (status & pwt->bit)
            ret = apr_pstrcat(p, ret, pwt->name, NULL);
        pwt++;
    }
    if (PROXY_WORKER_IS_USABLE(w))
        ret = apr_pstrcat(p, ret, "Ok ", NULL);
    return ret;
}

PROXY_DECLARE(apr_status_t) ap_proxy_sync_balancer(proxy_balancer *b, server_rec *s,
                                                    proxy_server_conf *conf)
{
    proxy_worker **workers;
    int i;
    int index;
    proxy_worker_shared *shm;
    proxy_balancer_method *lbmethod;
    ap_slotmem_provider_t *storage = b->storage;

    if (b->s->wupdated <= b->wupdated)
        return APR_SUCCESS;
    /* balancer sync */
    lbmethod = ap_lookup_provider(PROXY_LBMETHOD, b->s->lbpname, "0");
    if (lbmethod) {
        b->lbmethod = lbmethod;
    }
    /* worker sync */

    /*
     * Look thru the list of workers in shm
     * and see which one(s) we are lacking...
     * again, the cast to unsigned int is safe
     * since our upper limit is always max_workers
     * which is int.
     */
    for (index = 0; index < b->max_workers; index++) {
        int found;
        apr_status_t rv;
        if ((rv = storage->dptr(b->wslot, (unsigned int)index, (void *)&shm)) != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_EMERG, rv, s, APLOGNO(00965) "worker slotmem_dptr failed");
            return APR_EGENERAL;
        }
        /* account for possible "holes" in the slotmem
         * (eg: slots 0-2 are used, but 3 isn't, but 4-5 is)
         */
        if (!shm->hash.def || !shm->hash.fnv)
            continue;
        found = 0;
        workers = (proxy_worker **)b->workers->elts;
        for (i = 0; i < b->workers->nelts; i++, workers++) {
            proxy_worker *worker = *workers;
            if (worker->hash.def == shm->hash.def && worker->hash.fnv == shm->hash.fnv) {
                found = 1;
                break;
            }
        }
        if (!found) {
            proxy_worker **runtime;
            runtime = apr_array_push(b->workers);
            *runtime = apr_palloc(conf->pool, sizeof(proxy_worker));
            (*runtime)->hash = shm->hash;
            (*runtime)->context = NULL;
            (*runtime)->cp = NULL;
            (*runtime)->balancer = b;
            (*runtime)->s = shm;
            (*runtime)->tmutex = NULL;
            if ((rv = ap_proxy_initialize_worker(*runtime, s, conf->pool)) != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_EMERG, rv, s, APLOGNO(00966) "Cannot init worker");
                return rv;
            }
        }
    }
    if (b->s->need_reset) {
        if (b->lbmethod && b->lbmethod->reset)
            b->lbmethod->reset(b, s);
        b->s->need_reset = 0;
    }
    b->wupdated = b->s->wupdated;
    return APR_SUCCESS;
}

void proxy_util_register_hooks(apr_pool_t *p)
{
    APR_REGISTER_OPTIONAL_FN(ap_proxy_retry_worker);
}
