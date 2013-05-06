#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include "StringHelper.h"

using namespace std;
using namespace NSVirgo;

/////////////////////////////////////////////////////////////////////////////////

typedef string::size_type size_type;
//adjacency_list<OutEdgeList, VertexList, Directed, VertexProperties, EdgeProperties, GraphProperties, EdgeList>
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS> graph_type;

/////////////////////////////////////////////////////////////////////////////////

const bool g_Print = false;
string g_Program;
bool g_Verbose = false;
string g_CmdFile;
vector<pthread_t> g_vThread;
queue<string> g_qCommand;
pthread_mutex_t g_MutexQueue;
pthread_mutex_t g_MutexLog;
pthread_cond_t g_CondSync;
pthread_cond_t g_CondNotEmpty;
size_type g_AlreadySync;
string g_LogFile;
bool g_ErrorOccur = false;

/////////////////////////////////////////////////////////////////////////////////

void Usage(int argc, char* argv[])
{
    cerr << "Usage:" << endl;
    cerr << "    " << argv[0] << " CmdFile ThreadNum [OPTION]" << endl;
    cerr << "Function:" << endl;
    cerr << "    Read command from pipe file, multi-run commands." << endl;
    cerr << "Option:" << endl;
    cerr << "    CmdFile              The input command file." << endl;
    cerr << "    ThreadNum            The thread number to run." << endl;
    cerr << "        --help           Display this message and exit." << endl;
    cerr << "        --verbose        Verbose mode." << endl;
    cerr << "    -l, --log-file [F]   Output log file. If not specified, ignored." << endl;
    cerr << "Note:" << endl;
    cerr << "    Special commands begin with #:" << endl;
    cerr << "    #sync    Synchronize commands" << endl;
    cerr << "    #exit    End this multirun program." << endl;
    exit(1);
}

void LogFile(const string& content)
{
    static ofstream fout;
    if (g_LogFile.empty())
    {
        return;
    }
    if (content.empty())
    {
        return;
    }
    //int ret;
    //  lock g_MutexLog
    //ret = pthread_mutex_lock(&g_MutexLog);
    //if (ret != 0)
    //{
    //    cerr << "pthread_mutex_lock error: g_MutexLog: error=" << ret << endl;
    //    exit(1);
    //}
    //  open
    if (!fout.is_open())
    {
        fout.open(g_LogFile.c_str());
        if (!fout.good())
        {
            cerr << g_Program << ": open file error: " << g_LogFile << endl;
            exit(1);
        }
    }
    //  write
    if (!content.empty())
    {
        fout << content << endl;
    }
    ////  unlock g_MutexLog
    //ret = pthread_mutex_unlock(&g_MutexLog);
    //if (ret != 0)
    //{
    //    cerr << "pthread_mutex_unlock error: g_MutexLog: error=" << ret << endl;
    //    exit(1);
    //}
}

int ExecCommand(const string& cmd)
{
    return system(cmd.c_str());
}

void* ThreadFunction(void* arg)
{
    int ret;
    string cmd;
    bool runflag = true;
    size_type pid = *static_cast<size_type*>(arg);
    while (runflag)
    {
        cmd = "";
        //  lock g_MutexQueue
        ret = pthread_mutex_lock(&g_MutexQueue);
        if (ret != 0)
        {
            cerr << "pthread_mutex_lock error: g_MutexQueue: error=" << ret << endl;
            exit(1);
        }
        //  wait g_CondNotEmpty
        if (g_Print)
        {
            cerr << "thread " << pid << ": enter g_qCommand.empty()" << endl;
        }
        while (g_qCommand.empty())
        {
            if (g_Print)
            {
                cerr << "thread " << pid << ": enter pthread_cond_wait g_CondNotEmpty" << endl;
            }
            ret = pthread_cond_wait(&g_CondNotEmpty, &g_MutexQueue);
            if (ret != 0)
            {
                cerr << "pthread_cond_wait error: g_CondNotEmpty: error=" << ret << endl;
                exit(1);
            }
            if (g_Print)
            {
                cerr << "thread " << pid << ": leave pthread_cond_wait g_CondNotEmpty" << endl;
            }
        }
        if (g_Print)
        {
            cerr << "thread " << pid << ": leave g_qCommand.empty()" << endl;
        }
        //  get command
        assert(!g_qCommand.empty());
        cmd = g_qCommand.front();
        ostringstream log_oss;
        log_oss << "thread " << pid << ": get command: &" << cmd << "&";
        LogFile(log_oss.str().c_str());
        //  process
        if (cmd == "#exit")
        {
            runflag = false;
        }
        else if (cmd == "#sync")
        {
            ++g_AlreadySync;
            if (g_AlreadySync >= g_vThread.size())
            {
                //  last sync thread
                g_AlreadySync = 0;
                ret = pthread_cond_broadcast(&g_CondSync);
                if (ret != 0)
                {
                    cerr << "pthread_cond_broadcast error: g_CondSync: error=" << ret << endl;
                    exit(1);
                }
                g_qCommand.pop();
            }
            else
            {
                //  not last sync thread
                ret = pthread_cond_wait(&g_CondSync, &g_MutexQueue);
                if (ret != 0)
                {
                    cerr << "pthread_cond_wait error: g_CondSync: error=" << ret << endl;
                    exit(1);
                }
            }
        }
        else
        {
            g_qCommand.pop();
        }
        //  unlock g_MutexQueue
        ret = pthread_mutex_unlock(&g_MutexQueue);
        if (ret != 0)
        {
            cerr << "pthread_mutex_unlock error: g_MutexQueue: error=" << ret << endl;
            exit(1);
        }
        //  exec
        assert(!cmd.empty());
        if (cmd[0] != '#')
        {
            ostringstream log_oss;
            if (0 == ExecCommand(cmd.c_str()))
            {
                log_oss << "thread " << pid << ": execute done command: &" << cmd << "&";
            }
            else
            {
                log_oss << "thread " << pid << ": execute failed command: &" << cmd << "&";
                g_ErrorOccur = true;
            }
            LogFile(log_oss.str());
        }
    }
    if (g_Print)
    {
        cerr << "thread " << pid << ": leave ThreadFunction" << endl;
    }
    return NULL;
}

void InitOption(int argc, char* argv[])
{
    g_Program = argv[0];
    for (int i=1; i<argc; ++i)
    {
        const string arg = argv[i];
        if (arg == "--help")
        {
            Usage(argc, argv);
        }
        else if (arg == "--verbose")
        {
            g_Verbose = true;
        }
        else if (arg == "-l" || arg == "--log-file")
        {
            ++i;
            if (i >= argc)
            {
                cerr << argv[0] << ": missing argument for option " << arg << endl;
                exit(1);
            }
            g_LogFile = argv[i];
        }
        else if (arg[0] == '-')
        {
            cerr << argv[0] << ": invalid option: " << arg << endl;
            exit(1);
        }
        else
        {
            if (g_CmdFile.empty())
            {
                g_CmdFile = arg;
            }
            else if (g_vThread.empty())
            {
                int threadnum = atoi(arg.c_str());
                g_vThread.resize(threadnum, static_cast<pthread_t>(-1));
            }
            else 
            {
                cerr << argv[0] << ": extra operand: " << arg << endl;
                exit(1);
            }
        }
    }
    if (g_CmdFile.empty() || g_vThread.empty())
    {
        Usage(argc, argv);
    }
    g_AlreadySync = 0;
    ////  mkfifo
    //if (mkfifo(g_CmdFile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))
    //{
    //    if (errno == EEXIST)
    //    {
    //        cerr << "mkfifo error: file exist [ignored]: " << g_CmdFile << endl;
    //    }
    //    else
    //    {
    //        cerr << "mkfifo error: " << g_CmdFile << endl;
    //        exit(1);
    //    }
    //}
    if (g_Verbose)
    {
        cerr << "g_Verbose        : " << g_Verbose << endl;
        cerr << "g_CmdFile        : " << g_CmdFile << endl;
        cerr << "g_vThread.size() : " << g_vThread.size() << endl;
        cerr << "g_LogFile        : " << g_LogFile << endl;
    }
}

void InitThread()
{
    assert(!g_vThread.empty());
    int ret;
    size_type i;
    //  init mutex 
    ret = pthread_mutex_init(&g_MutexQueue, NULL);
    if (ret != 0)
    {
        cerr << "pthread_mutex_init error: g_MutexQueue: error=" << ret << endl;
        exit(1);
    }
    ret = pthread_mutex_init(&g_MutexLog, NULL);
    if (ret != 0)
    {
        cerr << "pthread_mutex_init error: g_MutexLog: error=" << ret << endl;
        exit(1);
    }
    //  init cond
    ret = pthread_cond_init(&g_CondSync, NULL);
    if (ret != 0)
    {
        cerr << "pthread_cond_init error: g_CondSync: error=" << ret << endl;
        exit(1);
    }
    ret = pthread_cond_init(&g_CondNotEmpty, NULL);
    if (ret != 0)
    {
        cerr << "pthread_cond_init error: g_CondNotEmpty: error=" << ret << endl;
        exit(1);
    }
    //  create thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    for (i=0; i<g_vThread.size(); ++i)
    {
        size_type* p = new size_type;
        *p = i;
        ret = pthread_create(&g_vThread[i], &attr, ThreadFunction, p);
        if (ret != 0)
        {
            cerr << "pthread_create error: error=" << ret << "    i=" << i << endl;
            exit(1);
        }
        ostringstream log_oss;
        log_oss << "main thread: create g_vThread[" << i << "]=" << g_vThread[i];
        LogFile(log_oss.str().c_str());
    }
}

void Uninit()
{
    size_type i;
    int ret;
    ////  unlink file
    //if (unlink(g_CmdFile.c_str()) != 0)
    //{
    //    cerr << "unlink error: " << g_CmdFile << endl;
    //    exit(1);
    //}
    //for (i=0; i<g_vThread.size(); ++i)
    //{
    //    ret = pthread_cancel(g_vThread[i]);
    //    if (ret != 0)
    //    {
    //        cerr << "pthread_cancel error: error=" << ret << endl;
    //        exit(1);
    //    }
    //}
    //  john thread
    for (i=0; i<g_vThread.size(); ++i)
    {
        ret = pthread_join(g_vThread[i], NULL);
        if (ret != 0)
        {
            cerr << "pthread_join error: error=" << ret << "    g_vThread[" << i << "]=" << g_vThread[i] << endl;
            exit(1);
        }
        ostringstream log_oss;
        log_oss << "main thread: joined g_vThread[" << i << "]=" << g_vThread[i];
        LogFile(log_oss.str());
    }
    //  destroy mutex
    ret = pthread_mutex_destroy(&g_MutexQueue);
    if (ret != 0)
    {
        cerr << "pthread_mutex_destroy error: g_MutexQueue: error=" << ret << endl;
        exit(1);
    }
    ret = pthread_mutex_destroy(&g_MutexLog);
    if (ret != 0)
    {
        cerr << "pthread_mutex_destroy error: g_MutexLog: error=" << ret << endl;
        exit(1);
    }
    //  destroy cond
    ret = pthread_cond_destroy(&g_CondSync);
    if (ret != 0)
    {
        cerr << "pthread_cond_destroy error: g_CondSync: error=" << ret << endl;
        exit(1);
    }
    ret = pthread_cond_destroy(&g_CondNotEmpty);
    if (ret != 0)
    {
        cerr << "pthread_cond_destroy error: g_CondNotEmpty: error=" << ret << endl;
        exit(1);
    }
    //  exit
    if (g_ErrorOccur)
    {
        LogFile("main thread: something wrong, anyway, I am exiting, bye");
    }
    else
    {
        LogFile("main thread: all threads exited normally, I am exiting, bye");
    }
}

void MainLoop()
{
    if (g_Print)
    {
        cerr << "=========mainloop=========" << endl;
    }
    int ret;
    bool runflag = true;
    while (runflag)
    {
        ifstream fin;
        fin.open(g_CmdFile.c_str());
        if (!fin.good())
        {
            cerr << "fin.open error: " << g_CmdFile << endl;
            exit(1);
        }
        string line;
        while (runflag && getline(fin, line))
        {
            //cout << "Command: " << line << endl;
            //  empty line
            NSStringHelper::Trim(line);
            if (line.empty())
            {
                continue;
            }
            if (line == "#exit")
            {
                runflag = false;
            }
            //  lock g_MutexQueue
            ret = pthread_mutex_lock(&g_MutexQueue);
            if (ret != 0)
            {
                cerr << "pthread_mutex_lock error: g_MutexQueue: error=" << ret << endl;
                exit(1);
            }
            //  push
            g_qCommand.push(line);
            //  unlock g_MutexQueue
            ret = pthread_mutex_unlock(&g_MutexQueue);
            if (ret != 0)
            {
                cerr << "pthread_mutex_unlock error: g_MutexQueue: error=" << ret << endl;
                exit(1);
            }
            //  signal g_CondNotEmpty
            if (runflag)
            {
                ret = pthread_cond_signal(&g_CondNotEmpty);
                if (ret != 0)
                {
                    cerr << "pthread_cond_signal error: g_CondNotEmpty: error=" << ret << endl;
                    exit(1);
                }
            }
            else
            {
                ret = pthread_cond_broadcast(&g_CondNotEmpty);
                if (ret != 0)
                {
                    cerr << "pthread_cond_broadcast error: g_CondNotEmpty: error=" << ret << endl;
                    exit(1);
                }
            }
        }
        fin.close();
    }
}

int main(int argc, char* argv[])
{
    InitOption(argc, argv);
    InitThread();
    MainLoop();
    Uninit();
    if (g_ErrorOccur)
    {
        return 1;
    }
    return 0;
}

