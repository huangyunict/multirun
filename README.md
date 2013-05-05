Multirun Project
================
This project provides function to multi-run list of shell commands, as well simple synchronization. <br />
这个工程提供多线程并行执行脚本命令序列的功能，以及简单的同步功能。

The whole project includes two standalone programs: `multirun` and `multictrl` (not implemented yet). <br />
整个工程包含两个可执行程序: `multirun` 和 `multictrl` (尚未完成)。

Currently, the thread library is the `pthread` library, but will be the new standard C++11 thread. <br />
目前线程库采用的是 `pthread` 线程库，但下一步会转向新标准 C++11 的线程库。


multirun
--------
The `multirun` is a standalone problem to multi-run given shell commands. <br />
程序 `multirun` 是一个多线程运行命令脚本的程序。

The general model of the multirun is the classic producer-consumer problem. <br />
程序的基本模型是经典的生产者消费者问题。

One main producer thread reads commands from a input file and submit it to task list. <br />
一个生产者主线程负责从输入文件中读取命令并提交任务到任务队列。

Multiple consumer threads get tasks from task list and run them concurrently. <br />
多个消费者线程从任务队列中取出命令并同时执行。

Due to the multi-threading nature, the actual executing order of commands are unknown in advance.
由于多线程的本质，各个命令的执行顺序是不可预知的。

The input command file and thread number must be given when `multirun` starts.
在 `multirun` 启动时，必须指定输入文件和启动的线程数。

You may use `-l LOG` to see what happened inside the `multirun` program.
你可以用 `-l LOG` 查看程序 `multirun` 的日志。


### Command file format
The input of `multirun` is a command file, one command per line. <br />
程序 `multirun` 的输入是一个文本文件, 一行是一个命令。

If you want to execute multi-line shell commands, please first wrap them in a single shell script file. <br /> 
如果需要执行跨行命令，请首先封装成脚本文件。

Lines starting with `#` and the contents of which are not special commands will be ignored. <br />
以 `#` 顶格开始且内容不是特殊命令的行会被忽略

The space characters at the beginning and the end of lines will be ignored.
命令行两端的空白字符会被忽略。

The commands in the input file should not wait for keyboard input. You may redirect standard input from file instead. <br />
用户命令不能等待从键盘输入数据，可以使用输入重定向实现输入。

The commands in the input file had better output nothing, otherwise the outputs may be mingled together. You may redirect standard output and standard error to files instead. <br />
用户命令最好不要带输出，否则输出会交错在一起，可以用重定向将命令的标准输出和错误输出重定向到文件。


### Special commands
*   The special barrier synchronization command: `#sync`. <br />
    特殊的路障同步命令: `#sync`. <br>
    All threads will be suspended when `#sync` is executed, and waked up until the last thread executes the command. <br />
    所有线程执行到命令 `#sync` 时会等待，直到最后一个线程执行到此才被唤醒。 <br>
    Current version of `multirun` only support simple barrier synchronization, and other complicated synchronizations are not supported yet. <br />
    目前版本的 `multirun` 只提供了简单的同步路障功能，暂不支持更为复杂的指定命令依赖关系的操作。

*   The special exiting command: `#exit`. <br />
    特殊的退出命令: `#exit`. <br />
    All commands after the exiting command will be ignored. <br />
    所有在退出命令之后的命令会被忽略。


