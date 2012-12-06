#include <stdarg.h>
#include <getopt.h>
#include "lsi_tool.h"
#include "lsi_ctl.h"

typedef struct LsiCmdFlag
{
    const char* cfgfile;
    const char* cmd;
} LsiCmdFlag;

const char* usage()
{
    static char buf[1024];
    memset(buf, 0, sizeof(buf));
    snprintf(buf,  sizeof(buf),  "usage: \n"\
             "	--conf:		LSI conifg file\n"\
             "	init | status | clean\n");
    return buf;
}

static const char* s_short_opts = "";
static const option s_long_opts[] =
{
    {"conf",    1,  NULL,   0},
    {NULL,  0,      NULL,   0},
};

int main(int argc, char** argv)
{
    int opt = 0;
    int long_index = 0;
    LsiCmdFlag flag;
    memset(&flag, 0, sizeof(flag));
    while ((opt = getopt_long(argc, argv, s_short_opts, s_long_opts,
                              &long_index)) != -1)
    {
        if (!strcmp("conf", s_long_opts[long_index].name))
        {
            flag.cfgfile = optarg;
        }
    }

    // Parse "init | clean | status".
    for (opt = optind; opt < argc; ++opt)
    {
        if (!strcmp("init", argv[opt]) ||
            !strcmp("clean", argv[opt]) ||
            !strcmp("status", argv[opt]))
        {
            flag.cmd = argv[opt];
            break;
        }
    }

    // check app flag valid
    if (!flag.cfgfile || !flag.cmd)
    {
        printf("%s\n", usage());
        exit(0);
    }

    // bus control data
    LsiCtl* lsi_ctl = lsi_ctl_create(flag.cfgfile);
    if (!lsi_ctl)
    {
        printf("read config file[%s] fail\n", flag.cfgfile);
        exit(0);
    }

    // init bus
    if (0 == strcmp("init", flag.cmd))
    {
        lsi_ctl_init(lsi_ctl);
    }
    // clean bus
    else if (0 == strcmp("clean", flag.cmd))
    {
        lsi_ctl_clean(lsi_ctl);
    }
    // show status
    else if (0 == strcmp("status", flag.cmd))
    {
        lsi_ctl_status(lsi_ctl);
    }

    // free memory
    lsi_ctl_destroy(lsi_ctl);
    return 0;
}


