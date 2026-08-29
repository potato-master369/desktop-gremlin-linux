#ifndef PAYLOAD_H
#define PAYLOAD_H
#define DEP_GCC 0
#define DEP_PKGCONFIG 1
#define DEP_GTK4 2
#define DEP_GTK4_LAYERSHELL 3
#define DEP_GIT 4

int depcheck(int id);
int clonegit(void);
int cd_src(void);
int build_src(void);
int install_src(void);
#endif
