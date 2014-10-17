#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "su_srv.h"

int main(int argc, char *args[])
{
    char *pfs_root = malloc(sizeof(char) * 255);
    getcwd(pfs_root, 255);

    if (su_srv_open_shell_session(pfs_root))
    {
        printf("failed to open SU shell session !\n");
    }
    else
    {
        printf("shell session opened\n"); 

        int rval = su_srv_exec("id");
        printf("id() exit code: %d\n", rval);

        rval = su_srv_exec("undefined_command");
        printf("undefined_command() exit code: %d\n", rval);

        if (su_srv_exit_shell_session())
            printf("failed to properly close shell session\n");
        else
            printf("shell session should have gracefully close\n");
    }

    free(pfs_root);
    return 0;
}
