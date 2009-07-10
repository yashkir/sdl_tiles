enum errorlevel { ERR_MINOR, ERR_FATAL };

/* Usage:
 * to cause exit: error(ERR_FATAL, "Some message %d\n", somevar);
 * to just print: error(ERR_MINOR, "Some message %d\n", somevar); */
void
error (enum errorlevel errorlevel, char *fmt, ...);
