dnl modules enabled in this directory by default

dnl APACHE_MODULE(name, helptext[, objects[, structname[, default[, config]]]])

APACHE_MODPATH_INIT(filters)

APACHE_MODULE(buffer, Filter Buffering, , , most)
APACHE_MODULE(data, RFC2397 data encoder, , , )
APACHE_MODULE(ratelimit, Output Bandwidth Limiting, , , most)
APACHE_MODULE(reqtimeout, Limit time waiting for request from client, , , yes)
APACHE_MODULE(ext_filter, external filter module, , , most)
APACHE_MODULE(request, Request Body Filtering, , , most)
APACHE_MODULE(include, Server Side Includes, , , most)
APACHE_MODULE(filter, Smart Filtering, , , yes)
APACHE_MODULE(reflector, Reflect request through the output filter stack, , , )
APACHE_MODULE(substitute, response content rewrite-like filtering, , , most)

sed_obj="mod_sed.lo sed0.lo sed1.lo regexp.lo"
APACHE_MODULE(sed, filter request and/or response bodies through sed, $sed_obj, , most, [
    if test "x$enable_sed" = "xshared"; then
        # The only symbol which needs to be exported is the module
        # structure, so ask libtool to hide libsed internals:
        APR_ADDTO(MOD_SED_LDADD, [-export-symbols-regex sed_module])
    fi
])

if test "$ac_cv_ebcdic" = "yes"; then
# mod_charset_lite can be very useful on an ebcdic system,
#   so include it by default
    APACHE_MODULE(charset_lite, character set translation.  Enabled by default only on EBCDIC systems., , , yes)
else
    APACHE_MODULE(charset_lite, character set translation.  Enabled by default only on EBCDIC systems., , , )
fi


APACHE_MODULE(deflate, Deflate transfer encoding support, , , most, [
  AC_ARG_WITH(z, APACHE_HELP_STRING(--with-z=DIR,use a specific zlib library),
  [
    if test "x$withval" != "xyes" && test "x$withval" != "x"; then
      ap_zlib_base="$withval"
      ap_zlib_with="yes"
    fi
  ])
  if test "x$ap_zlib_base" = "x"; then
    AC_MSG_CHECKING([for zlib location])
    AC_CACHE_VAL(ap_cv_zlib,[
      for dir in /usr/local /usr ; do
        if test -d $dir && test -f $dir/include/zlib.h; then
          ap_cv_zlib=$dir
          break
        fi
      done
    ])
    ap_zlib_base=$ap_cv_zlib
    if test "x$ap_zlib_base" = "x"; then
      enable_deflate=no
      AC_MSG_RESULT([not found])
    else
      AC_MSG_RESULT([$ap_zlib_base])
    fi
  fi
  if test "$enable_deflate" != "no"; then
    ap_save_includes=$INCLUDES
    ap_save_ldflags=$LDFLAGS
    ap_save_cppflags=$CPPFLAGS
    ap_zlib_ldflags=""
    if test "$ap_zlib_base" != "/usr"; then
      APR_ADDTO(INCLUDES, [-I${ap_zlib_base}/include])
      dnl put in CPPFLAGS temporarily so that AC_TRY_LINK below will work
      CPPFLAGS="$CPPFLAGS $INCLUDES"
      APR_ADDTO(LDFLAGS, [-L${ap_zlib_base}/lib])
      APR_ADDTO(ap_zlib_ldflags, [-L${ap_zlib_base}/lib])
      if test "x$ap_platform_runtime_link_flag" != "x"; then
         APR_ADDTO(LDFLAGS, [$ap_platform_runtime_link_flag${ap_zlib_base}/lib])
         APR_ADDTO(ap_zlib_ldflags, [$ap_platform_runtime_link_flag${ap_zlib_base}/lib])
      fi
    fi
    APR_ADDTO(LIBS, [-lz])
    AC_MSG_CHECKING([for zlib library])
    AC_TRY_LINK([#include <zlib.h>], [int i = Z_OK;], 
      [AC_MSG_RESULT(found) 
       APR_ADDTO(MOD_DEFLATE_LDADD, [$ap_zlib_ldflags -lz])],
      [AC_MSG_RESULT(not found)
       enable_deflate=no
       INCLUDES=$ap_save_includes
       if test "x$ap_zlib_with" = "x"; then
         AC_MSG_WARN([... Error, zlib was missing or unusable])
       else
         AC_MSG_ERROR([... Error, zlib was missing or unusable])
       fi
      ])
    LDFLAGS=$ap_save_ldflags
    CPPFLAGS=$ap_save_cppflags
    APR_REMOVEFROM(LIBS, [-lz])
  fi
])

AC_DEFUN(FIND_LIBXML2, [
  AC_CACHE_CHECK([for libxml2], [ac_cv_libxml2], [
    AC_ARG_WITH(libxml2,
      [APACHE_HELP_STRING(--with-libxml2,location for libxml2)],
      [test_paths="${with_libxml2}"],
      [test_paths="/usr/include/libxml2 /usr/local/include/libxml2 /usr/include /usr/local/include"]
    )
    AC_MSG_CHECKING(for libxml2)
    xml2_path=""
    for x in ${test_paths}; do
        if test -f "${x}/libxml/parser.h"; then
          xml2_path="${x}"
          break
        fi
    done
    if test -n "${xml2_path}" ; then
      ac_cv_libxml2=yes
      XML2_INCLUDES="${xml2_path}"
    else
      ac_cv_libxml2=no
    fi
  ])
])

APACHE_MODULE(xml2enc, i18n support for markup filters, , , , [
  FIND_LIBXML2
  if test "$ac_cv_libxml2" = "yes" ; then
    APR_ADDTO(CFLAGS, [-I${XML2_INCLUDES}])
    APR_ADDTO(MOD_XML2ENC_LDADD, [-lxml2])
  else
    enable_xml2enc=no
  fi
])
APACHE_MODULE(proxy_html, Fix HTML Links in a Reverse Proxy, , , , [
  FIND_LIBXML2
  if test "$ac_cv_libxml2" = "yes" ; then
    APR_ADDTO(CFLAGS, [-I${XML2_INCLUDES}])
    APR_ADDTO(MOD_PROXY_HTML_LDADD, [-lxml2])
  else
    enable_proxy_html=no
  fi
]
)

APR_ADDTO(INCLUDES, [-I\$(top_srcdir)/$modpath_current])

APACHE_MODPATH_FINISH
