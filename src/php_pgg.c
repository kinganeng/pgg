
/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   +----------------------------------------------------------------------+
*/

#include "pgg.h"

#include "php.h"
#include "php_globals.h"
#include "php_variables.h"
#include "zend_modules.h"
#include "zend_ini_scanner.h"
#include "zend_stream.h"

#include "SAPI.h"

#include "zend.h"
#include "zend_extensions.h"
#include "php_ini.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_highlight.h"
#include "zend_indent.h"

#include "php_getopt.h"

#include <php_config.h>

static int pgg_is_running = 0;

#define PHP_MODE_STANDARD    1
#define PHP_MODE_HIGHLIGHT    2
#define PHP_MODE_INDENT        3
#define PHP_MODE_LINT        4
#define PHP_MODE_STRIP        5

static char *php_optarg = NULL;
static int php_optind = 1;
static zend_module_entry pgg_module_entry;

static const opt_struct OPTIONS[] = {
    {'c', 1, "php-ini"},
    {'d', 1, "define"},
    {'e', 0, "profile-info"},
    {'h', 0, "help"},
    {'i', 0, "info"},
    {'m', 0, "modules"},
    {'n', 0, "no-php-ini"},
    {'?', 0, "usage"},/* help alias (both '?' and 'usage') */
    {'v', 0, "version"},
    {'s', 1, "pgg-config"},
    {'t', 0, "test"},
    {'p', 1, "prefix"},
    {'-', 0, NULL} /* end of args */
};


static int print_module_info(zend_module_entry *module, void *arg TSRMLS_DC) {
    php_printf("%s\n", module->name);
    return 0;
}


static int module_name_cmp(const void *a, const void *b TSRMLS_DC) {
    Bucket *f = *((Bucket **) a);
    Bucket *s = *((Bucket **) b);

    return strcasecmp(    ((zend_module_entry *)f->pData)->name,
                        ((zend_module_entry *)s->pData)->name);
}


static void print_modules(TSRMLS_D) {
    HashTable sorted_registry;
    zend_module_entry tmp;

    zend_hash_init(&sorted_registry, 50, NULL, NULL, 1);
    zend_hash_copy(&sorted_registry, &module_registry, NULL, &tmp, sizeof(zend_module_entry));
    zend_hash_sort(&sorted_registry, zend_qsort, module_name_cmp, 0 TSRMLS_CC);
    zend_hash_apply_with_argument(&sorted_registry, (apply_func_arg_t) print_module_info, NULL TSRMLS_CC);
    zend_hash_destroy(&sorted_registry);
}

static int print_extension_info(zend_extension *ext, void *arg TSRMLS_DC) {
    php_printf("%s\n", ext->name);
    return 0;
}

static int extension_name_cmp(const zend_llist_element **f, const zend_llist_element **s TSRMLS_DC) {
    return strcmp(    ((zend_extension *)(*f)->data)->name,
                    ((zend_extension *)(*s)->data)->name);
}

static void print_extensions(TSRMLS_D) {
    zend_llist sorted_exts;

    zend_llist_copy(&sorted_exts, &zend_extensions);
    sorted_exts.dtor = NULL;
    zend_llist_sort(&sorted_exts, extension_name_cmp TSRMLS_CC);
    zend_llist_apply_with_argument(&sorted_exts, (llist_apply_with_arg_func_t) print_extension_info, NULL TSRMLS_CC);
    zend_llist_destroy(&sorted_exts);
}


#ifndef STDOUT_FILENO     
#define STDOUT_FILENO 1     
#endif


static inline size_t sapi_pgg_single_write(const char *str, uint str_length TSRMLS_DC) {
    write(STDOUT_FILENO, str, str_length);
    return str_length;
}


static int php_pgg_startup(sapi_module_struct *sapi_module) /* {{{ */
{
    if (php_module_startup(sapi_module, &pgg_module_entry, 1)==FAILURE) {
        return FAILURE;
    }
    return SUCCESS;
}

static int sapi_pgg_activate(TSRMLS_D) {
	/* PATH_TRANSLATED should be defined at this stage but better safe than sorry :) */
	if (!SG(request_info).path_translated) {
		return FAILURE;
	}

	return SUCCESS;
}


static int sapi_pgg_deactivate(TSRMLS_D) {
    //fflush(stdout);
    if(SG(request_info).argv0) {
        free(SG(request_info).argv0);
        SG(request_info).argv0 = NULL;
    }
    return SUCCESS;
}


static int sapi_pgg_ub_write(const char *str, uint str_length TSRMLS_DC) {
    const char *ptr = str;
    uint remaining = str_length;
    size_t ret;

    while (remaining > 0) {
        ret = sapi_pgg_single_write(ptr, remaining TSRMLS_CC);
        if (!ret) {
            php_handle_aborted_connection();
            return str_length - remaining;
        }
        ptr += ret;
        remaining -= ret;
    }

    return str_length;
}


static void sapi_pgg_flush(void *server_context) {
}


static char *sapi_pgg_getenv(char *name, size_t name_len TSRMLS_DC) {
	/* if pgg has not started yet, use std env */
	return getenv(name);
}


static int sapi_pgg_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	 * header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}


static int sapi_pgg_read_post(char *buffer, uint count_bytes TSRMLS_DC) {
    uint read_bytes = 0;
    int tmp_read_bytes;

    return read_bytes;
}


static char *sapi_pgg_read_cookies(TSRMLS_D) {
    return NULL;
}


static void sapi_pgg_register_variables(zval *track_vars_array TSRMLS_DC) {
}


static void sapi_pgg_log_message(char *message TSRMLS_DC) {
    printf("log message:%s\n", message);
}


/* {{{ sapi_module_struct pgg_sapi_module
 */
static sapi_module_struct pgg_sapi_module = {
    "php-pgg",                          /* name */
    "PGG/EasyGameServer",               /* pretty name */

    php_pgg_startup,                    /* startup */
    php_module_shutdown_wrapper,        /* shutdown */

    sapi_pgg_activate,                  /* activate */
    sapi_pgg_deactivate,                /* deactivate */

    sapi_pgg_ub_write,                  /* unbuffered write */
    sapi_pgg_flush,                     /* flush */
    NULL,                               /* get uid */
    sapi_pgg_getenv,                    /* getenv */

    php_error,                          /* error handler */

    NULL,                               /* header handler */
    sapi_pgg_send_headers,              /* send headers handler */
    NULL,                               /* send header handler */

    sapi_pgg_read_post,                 /* read POST data */
    sapi_pgg_read_cookies,              /* read Cookies */

    sapi_pgg_register_variables,        /* register server variables */
    sapi_pgg_log_message,               /* Log message */
    NULL,                               /* Get request time */
    NULL,                               /* Child terminate */

    STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */


/* {{{ arginfo ext/standard/dl.c */
ZEND_BEGIN_ARG_INFO(arginfo_dl, 0)
    ZEND_ARG_INFO(0, extension_filename)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry additional_functions[] = {
	ZEND_FE(dl, arginfo_dl)
	{NULL, NULL, NULL}
};


static PHP_MINIT_FUNCTION(pgg)
{
    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(pgg)
{
    return SUCCESS;
}

static PHP_MINFO_FUNCTION(pgg) 
{
    php_info_print_table_start();
    php_info_print_table_row(2, "php-pgg", "active");
    php_info_print_table_end();
}


static const zend_function_entry pgg_sapi_functions[] = {
    {NULL, NULL, NULL}
};

static zend_module_entry pgg_module_entry = {
    STANDARD_MODULE_HEADER,
    "php-pgg",
    pgg_sapi_functions,
    PHP_MINIT(pgg),
    PHP_MSHUTDOWN(pgg),
    NULL,
    NULL,
    PHP_MINFO(pgg),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};


/* {{{ php_pgg_usage
 */
static void php_pgg_usage(char *argv0) {
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "php";
	}

	php_printf(	"Usage: %s [-n] [-e] [-h] [-i] [-m] [-v] [-t] [-p <prefix>] [-g <pid>] [-c <file>] [-d foo[=bar]] [-s <file>]\n"
				"  -c <path>|<file> Look for php.ini file in this directory\n"
				"  -n               No php.ini file will be used\n"
				"  -d foo[=bar]     Define INI entry foo with value 'bar'\n"
				"  -e               Generate extended information for debugger/profiler\n"
				"  -h               This help\n"
				"  -i               PHP information\n"
				"  -m               Show compiled in modules\n"
				"  -v               Version number\n"
				"  -p, --prefix <dir>\n"
				"                   Specify alternative prefix path to FastCGI process manager (default: %s).\n"
				"  -s, --pgg-config <file>\n"
				"                   Specify alternative path to easy game server config file.\n"
				"  -t, --test       Test PGG configuration and exit\n",
				prog, PHP_PREFIX);
}

int main(int argc, char *argv[]) {
	int exit_status = PGG_EXIT_OK;
	zend_file_handle file_handle;
    int c;

	/* temporary locals */
	int orig_optind = php_optind;
	char *orig_optarg = php_optarg;
	int ini_entries_len = 0;
	/* end of temporary locals */

	char *pgg_config = NULL;
	char *pgg_prefix = NULL;
	int test_conf = 0;
	int php_information = 0;
    int use_extended_info = 0;

#ifdef ZTS
	void ***tsrm_ls;
#endif

#ifdef ZTS
	tsrm_startup(1, 1, 0, NULL);
	tsrm_ls = ts_resource(0);
#endif

	sapi_startup(&pgg_sapi_module);
	pgg_sapi_module.php_ini_path_override = NULL;
	pgg_sapi_module.php_ini_ignore_cwd = 1;

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
		switch (c) {
			case 'c':
				if (pgg_sapi_module.php_ini_path_override) {
					free(pgg_sapi_module.php_ini_path_override);
				}
				pgg_sapi_module.php_ini_path_override = strdup(php_optarg);
				break;

			case 'n':
				pgg_sapi_module.php_ini_ignore = 1;
				break;

			case 'd': {
				/* define ini entries on command line */
				int len = strlen(php_optarg);
				char *val;

				if ((val = strchr(php_optarg, '='))) {
					val++;
					if (!isalnum(*val) && *val != '"' && *val != '\'' && *val != '\0') {
						pgg_sapi_module.ini_entries = realloc(pgg_sapi_module.ini_entries, ini_entries_len + len + sizeof("\"\"\n\0"));
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len, php_optarg, (val - php_optarg));
						ini_entries_len += (val - php_optarg);
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len, "\"", 1);
						ini_entries_len++;
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len, val, len - (val - php_optarg));
						ini_entries_len += len - (val - php_optarg);
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len, "\"\n\0", sizeof("\"\n\0"));
						ini_entries_len += sizeof("\n\0\"") - 2;
					} else {
						pgg_sapi_module.ini_entries = realloc(pgg_sapi_module.ini_entries, ini_entries_len + len + sizeof("\n\0"));
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len, php_optarg, len);
						memcpy(pgg_sapi_module.ini_entries + ini_entries_len + len, "\n\0", sizeof("\n\0"));
						ini_entries_len += len + sizeof("\n\0") - 2;
					}
				} else {
					pgg_sapi_module.ini_entries = realloc(pgg_sapi_module.ini_entries, ini_entries_len + len + sizeof("=1\n\0"));
					memcpy(pgg_sapi_module.ini_entries + ini_entries_len, php_optarg, len);
					memcpy(pgg_sapi_module.ini_entries + ini_entries_len + len, "=1\n\0", sizeof("=1\n\0"));
					ini_entries_len += len + sizeof("=1\n\0") - 2;
				}
				break;
			}

			case 'g':
				pgg_config = php_optarg;
				break;

			case 'p':
				pgg_prefix = php_optarg;
				break;

			case 'e': /* enable extended info output */
				use_extended_info = 1;
				break;

			case 't': 
				test_conf++;
				break;

			case 'm': /* list compiled in modules */
				pgg_sapi_module.startup(&pgg_sapi_module);
				php_output_activate(TSRMLS_C);
				SG(headers_sent) = 1;
				php_printf("[PHP Modules]\n");
				print_modules(TSRMLS_C);
				php_printf("\n[Zend Modules]\n");
				print_extensions(TSRMLS_C);
				php_printf("\n");
				php_output_end_all(TSRMLS_C);
				php_output_deactivate(TSRMLS_C);
				exit_status = PGG_EXIT_OK;
				goto out;

			case 'i': /* php info & quit */
                php_information = 1;
				break;
			case 'h':
			case '?':
				pgg_sapi_module.startup(&pgg_sapi_module);
				php_output_activate(TSRMLS_C);
				SG(headers_sent) = 1;
				php_pgg_usage(argv[0]);
				php_output_end_all(TSRMLS_C);
				php_output_deactivate(TSRMLS_C);
				exit_status = (c == 'h') ? PGG_EXIT_OK : PGG_EXIT_USAGE;
				goto out;

			case 'v': /* show php version & quit */
				pgg_sapi_module.startup(&pgg_sapi_module);
				if (php_request_startup(TSRMLS_C) == FAILURE) {
					SG(server_context) = NULL;
					php_module_shutdown(TSRMLS_C);
					return PGG_EXIT_SOFTWARE;
				}
				SG(headers_sent) = 1;
				SG(request_info).no_headers = 1;

#if ZEND_DEBUG
				php_printf("PHP %s (%s) (built: %s %s) (DEBUG)\nCopyright (c) 1997-2013 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__,        __TIME__, get_zend_version());
#else
				php_printf("PHP %s (%s) (built: %s %s)\nCopyright (c) 1997-2013 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__, __TIME__,      get_zend_version());
#endif
				php_request_shutdown((void *) 0);
				exit_status = PGG_EXIT_OK;
				goto out;
		}
	}


	if (php_information) {
        pgg_sapi_module.phpinfo_as_text = 1;
        pgg_sapi_module.startup(&pgg_sapi_module);
        if (php_request_startup(TSRMLS_C) == FAILURE) {
            SG(server_context) = NULL;
            php_module_shutdown(TSRMLS_C);
            return PGG_EXIT_SOFTWARE;
        }
        SG(headers_sent) = 1;
        SG(request_info).no_headers = 1;
        php_print_info(0xFFFFFFFF TSRMLS_CC);
        php_request_shutdown((void *) 0);
        exit_status = PGG_EXIT_OK;
        goto out;
    }
	
	if (use_extended_info) {
		CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
	}
    

    pgg_log_stderr(0, "hello world!\n");

out:

	SG(server_context) = NULL;
    php_module_shutdown(TSRMLS_C);
    sapi_shutdown();

#ifdef ZTS
	tsrm_shutdown();
#endif

	return exit_status;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
