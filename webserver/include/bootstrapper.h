#ifndef _WWM_BOOTSTRAPPER_H_
#define _WWM_BOOTSTRAPPER_H_

typedef struct wwm_bootstrapper_t_ *wwm_bootstrapper_t;

wwm_bootstrapper_t wwm_boostrapper_new();
void               wwm_bootstrapper_destroy(wwm_bootstrapper_t bs);

bool wwm_bootstrapper_parse_cmdline(wwm_bootstrapper_t bs, int argc, char **argv);
bool wwm_bootstrapper_configure(wwm_bootstrapper_t bs);
void wwm_bootstrapper_run(wwm_bootstrapper_t bs);

#endif // _WWM_BOOTSTRAPPER_H_
