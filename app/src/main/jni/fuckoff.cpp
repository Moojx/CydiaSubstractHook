#include <string.h>
#include <iostream>
#include <android/log.h>
#include <sys/atomics.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "Method.h"
#include <asm/user.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <dirent.h>
#include "substrate.h"
#include <jni.h>
#include <vector>
//#include <sys/types.h>
#include <sys/stat.h>   //makedir

using namespace std;

#define TAG "jx"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

const char *ex0 = "re-initialized>";
const char *ex1 = "zygote";
const char *ex2 = "app_process";
const char *ex3 = "/system/bin/dexopt";
const char *ex4 = "com.google.android.gms";
const char *ex5 = "com.google.android.gms.persistent";
const char *ex6 = "com.google.process.gapps";
const char *ex7 = "com.google.android.gms.wearable";
const char *ex8 = "com.android.phone";
const char *ex9 = "com.android.systemui";
const char *ex10 = "com.google.android.gms.unstable";
const char *ex11 = "android.process.acore";
const char *ex12 = "android.process.media";
const char *ex13 = "dexopt";

#define BUF_SIZE 1024

//hook系统库
//MSConfig(MSFilterLibrary, "libdvm.so");
//hook第三方应用库
MSConfig(MSFilterExecutable, "/system/bin/app_process");
//const char *workDir = "/mnt/sdcard/hookLua/";
vector<string> split(const string& str, const string& delim) ;

void getNameByPid(pid_t pid, char *task_name) {
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];
    sprintf(proc_pid_path, "/proc/%d/status", pid);
    FILE *fp = fopen(proc_pid_path, "r");
    if (NULL != fp) {
        if (fgets(buf, BUF_SIZE - 1, fp) == NULL) {
            fclose(fp);
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
}

int exclude(char *s) {
    int i = !strcmp(s, ex0) || !strcmp(s, ex1) || !strcmp(s, ex2) || !strcmp(s, ex3) ||
            !strcmp(s, ex4) || !strcmp(s, ex5) || !strcmp(s, ex6) || !strcmp(s, ex7) ||
            !strcmp(s, ex8) || !strcmp(s, ex9) || !strcmp(s, ex10) || !strcmp(s, ex11) ||
            !strcmp(s, ex12) || !strcmp(s, ex13);
    return i;
}

int checkDir(const char *fileExt) {
    mode_t myMode = 777;
    if (0 == access(fileExt, 0)) {//目录存在
        return 0;
    } else {
        if (0 == mkdir(fileExt, myMode)) {
            return 0;
        } else {
            return 1;
        }
    }
}

void (*old_dvmUseJNIBridge)(Method *, void *);

//Hook dvmUseJNIBridge函数
void new_dvmUseJNIBridge(Method *method, void *func) {
    char pname[50];
    pid_t pid = getpid();
    getNameByPid(pid, pname);

    if (exclude(pname)) {
        LOGI("exclude process:%s", pname);
        return old_dvmUseJNIBridge(method, func);
    }
    LOGD("hook dvmUseJNIBridge  start");
    LOGD("my_dvmUseJNIBridge class-%s ->name=%s  address=%08X", method->clazz->descriptor,
         method->name, func);
    //LOGD("my_dvmUseJNIBridge nativeFunc-%s", method->nativeFunc);
    if (0 == strcmp(method->name, "oncreat")) {
        sleep(9); //等待附加调试器
    }
    return old_dvmUseJNIBridge(method, func);
}

//**********************************************************************************
void replace_all(std::string &s, std::string const &t, std::string const &w) {
    string::size_type pos = s.find(t), t_size = t.size(), r_size = w.size();
    while (pos != std::string::npos) { // found
        s.replace(pos, t_size, w);
        pos = s.find(t, pos + r_size);
    }
}

/*
string getNextFilePath(const char *fileExt) {
    char buff[100] = {0};
    sprintf(buff, "/sdcard/hookLua/%s", fileExt);
    return buff;
}

void saveFile(const char *data, int len, const char *outFileName) {
    FILE *file = fopen(outFileName, "wb+");
    if (file != NULL) {
        fwrite(data, len, 1, file);
        fflush(file);
        fclose(file);
        chmod(outFileName, S_IRWXU | S_IRWXG | S_IRWXO);
        LOGD("[dumpcacas2d] allready dump lua with name: %s", outFileName);
    } else {
        LOGD("[%s] fopen failed, error: %s", __FUNCTION__, dlerror());
    }
}


static string g_strDataPath;
static int g_nCount = 1;

string getNextFilePath(const char *fileExt) {
    char buff[100] = {0};
    ++g_nCount;
    sprintf(buff, "/sdcard/hookLua/%d%s",g_nCount,  fileExt);
    return buff;
}
*/
// 分割字符串
vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    //判空处理
    if("" == str) return res;
    //先将要切割的字符串从string类型转换为char*类型
    char * strs = new char[str.length() + 1] ; //不要忘了
    strcpy(strs, str.c_str());

    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while(p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }
    return res;
}
//***********************************************************************************************
/*保存文件
param1 文件流
param2 文件大小
param3 文件名称*/
void saveFile(const void*  data ,int data_len,const char* name){
    //声明变量
    char allname[100];
    //清空残留数据
    memset(allname, 0, sizeof(allname) );
    //拼接头字符串
    strcat(allname,"/sdcard/hookLua/");
    //拼接文件名字带扩展名
    strcat(allname,name);
    LOGD("[dumpLua] dumped lua file success!: %s",allname);
    //写出文件
    FILE* outfile;
    outfile = fopen(allname,"wb");//(输入流) （变量）（输出文件流）
    fwrite(data , sizeof(unsigned char) , data_len , outfile);
    //清空
    fflush(outfile);
    //关闭文件流
    fclose(outfile);
    //free(allname);
    //free(data);
}

int (*old_luaL_loadbuffer)(void *L, const char *buff, int size, const char *name) = NULL;

//local function
int new_luaL_loadbuffer(void *L, const char *buff, int size, const char *name) {
    /*//打印lua名字与内容
    //LOGD("[dumplua] luaL_loadbuffer name: %s lua: %s", name, buff);
    //输出名字
    LOGD("[dumplua] luaL_loadbuffer name: %s", name);
    LOGD("*************************************************");
    //将分割的字符串存放到数组中
    std::vector<string> res = split(name, "/");
    //循环判断字符串是否包含.lua
    for (int i = 0; i < res.size(); ++i)
        //if(strstr(res[i].c_str(),".dll")){
        if(strstr(res[i].c_str(),".lua")){
            LOGD("[dumplua] lua name is, name: %s",res[i].c_str());
            //假如包含.lua则保存
            saveFile(buff, size,res[i].c_str());
        }
    return old_luaL_loadbuffer(L, buff, size, name);*/
   if (name != NULL) {
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/hookLua/";
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir, name_t);
                    //lua脚本保存
                     /*    file = fopen(full_name, "wb");
                         if(file!=NULL){
                           fwrite(buff,1,size,file);
                             fclose(file);
                             free(name_t);
                         }*/
                    //lua脚本hook加载
                    file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGD("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return old_luaL_loadbuffer(L, buff, size, name);
                    }
                }
            }
        }
    }
    return old_luaL_loadbuffer(L, buff, size, name);
}

int (*old_mono)(char *data, int data_len, int need_copy, void *status, int refonly, const char *name) = NULL;
int new_mono(char *data, int data_len, int need_copy, void *status, int refonly, const char *name) {
    LOGD("[dumpu3d] dll details is->   name: %s, len: %d, buff: %s", name, data_len, data);
    int ret = old_mono(data, data_len, need_copy, status, refonly, name);
    if(strstr(name,"Assembly-CSharp.dll")){
        LOGD("[dumpu3d] dumped dll successful! : %s ", name);
        saveFile(data, data_len, name);
    } else{
        return ret;
    }
    return ret;
}

//获取so基地址
void * get_base_of_lib_from_maps(char *soname)
{
    void *imagehandle = dlopen(soname, RTLD_LOCAL | RTLD_LAZY);
    if (soname == NULL)
        return NULL;
    if (imagehandle == NULL){
        return NULL;
    }
    uintptr_t * irc = NULL;
    FILE *f = NULL;
    char line[200] = {0};
    char *state = NULL;
    char *tok = NULL;
    char * baseAddr = NULL;
    if ((f = fopen("/proc/self/maps", "r")) == NULL)
        return NULL;
    while (fgets(line, 199, f) != NULL)
    {
        tok = strtok_r(line, "-", &state);
        baseAddr = tok;
        tok = strtok_r(NULL, "\t ", &state);
        tok = strtok_r(NULL, "\t ", &state); // "r-xp" field
        tok = strtok_r(NULL, "\t ", &state); // "0000000" field
        tok = strtok_r(NULL, "\t ", &state); // "01:02" field
        tok = strtok_r(NULL, "\t ", &state); // "133224" field
        tok = strtok_r(NULL, "\t ", &state); // path field

        if (tok != NULL) {
            int i;
            for (i = (int)strlen(tok)-1; i >= 0; --i) {
                if (!(tok[i] == ' ' || tok[i] == '\r' || tok[i] == '\n' || tok[i] == '\t'))
                    break;
                tok[i] = 0;
            }
            {
                size_t toklen = strlen(tok);
                size_t solen = strlen(soname);
                if (toklen > 0) {
                    if (toklen >= solen && strcmp(tok + (toklen - solen), soname) == 0) {
                        fclose(f);
                        return (uintptr_t*)strtoll(baseAddr,NULL,16);
                    }
                }
            }
        }
    }
    fclose(f);
    return NULL;
}
void * get_base_of_lib_from_soinfo(char *soname)
{
    if (soname == NULL)
        return NULL;
    void *imagehandle = dlopen(soname, RTLD_LOCAL | RTLD_LAZY);
    if (imagehandle == NULL){
        return NULL;
    }
    char *basename;
    char *searchname;
    int i;
    void * libdl_ptr=dlopen("libdl.so",3);
    basename = strrchr(soname,'/');
    searchname = basename ? basename +1 : soname;
    for(i =(int) libdl_ptr; i!=NULL; i=*(int*)(i+164)){
        if(!strcmp(searchname,(char*)i)){
            unsigned int *lbase= (unsigned int*)i+140;
            void * baseaddr = (void*)*lbase;
            return baseaddr;
        }
    }
    return NULL;

}
//获取模块名称及导出函数
void *lookup_symbol(char *libraryname, char *symbolname) {
    void *imagehandle = dlopen(libraryname, RTLD_GLOBAL | RTLD_NOW);
    if (imagehandle != NULL) {
        void *sym = dlsym(imagehandle, symbolname);
        if (sym != NULL) {
            return sym;
        } else {
            LOGI("(lookup_symbol) dlsym didn't work");
            return NULL;
        }
    } else {
        LOGI("(lookup_symbol) dlerror: %s", dlerror());
        return NULL;
    }
}

MSInitialize {
    LOGD("Substrate initialized.");
    //void *dvmload = lookup_symbol("/data/data/com.lg.tycq.uc/lib/libcocos2dlua.so","luaL_loadbuffer");
    //void *dvmload = lookup_symbol("/data/data/com.tencent.tmgp.asura/lib/libmono.so","mono_image_open_from_data_with_name");
    //void *dvmload = lookup_symbol("/data/data/com.feamber.spaceracing.tecent/libs/libracing.so","luaL_loadbufferx");
    void *dvmload = lookup_symbol("/data/data/com.fangkuai.n1/lib/libgame.so222","luaL_loadbuffer");
    if (dvmload == NULL) {
        LOGD("error find luaL_loadbuffer.");
    } else {
        //LOGD("getMono() should be at %p. Let's hook it",getMono);
        MSHookFunction(dvmload, (void *) &new_luaL_loadbuffer, (void **) &old_luaL_loadbuffer);
        LOGD("hook luaL_loadbuffer sucess.");
    }
}