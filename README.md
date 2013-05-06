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

Due to the multi-threading nature, the actual executing order of commands are unknown in advance. <br />
由于多线程的本质，各个命令的执行顺序是不可预知的。

The input command file and thread number must be given when `multirun` starts. <br />
在 `multirun` 启动时，必须指定输入文件和启动的线程数。

You may use `-l LOG` to see what happened inside the `multirun` program. <br />
你可以用 `-l LOG` 查看程序 `multirun` 的日志。

**The input file could be a regular file or a FIFO, but in either case it must ends with `#exit` line, otherwise `multirun` will read the input file over and over again. This is a known bug.** <br />
**输入文件可以是普通文件或者FIFO，但无论那种情况输入文件必须以 `#exit` 结束，否则 `multirun` 会一遍又一遍地执行文件里的命令，这是一个已知的Bug。**

The exiting status of `multirun` is 0 if all input commands executed successfully, 1 if at least one input command failed.
如果所有输入命令都执行成功，程序 `multirun` 的退出状态为0，否则退出状态为1。


### Command file format
The input of `multirun` is a command file, one command per line. <br />
程序 `multirun` 的输入是一个文本文件, 一行是一个命令。

If you want to execute multi-line shell commands, please first wrap them in a single shell script file. <br /> 
如果需要执行跨行命令，请首先封装成脚本文件。

Lines starting with `#` and the contents of which are not special commands will be ignored. <br />
以 `#` 顶格开始且内容不是特殊命令的行会被忽略

The space characters at the beginning and the end of lines will be ignored. <br />
命令行两端的空白字符会被忽略。

The commands in the input file should not wait for keyboard input. You may redirect standard input from file instead. <br />
用户命令不能等待从键盘输入数据，可以使用输入重定向实现输入。

The commands in the input file had better output nothing, otherwise the outputs may be mingled together. You may redirect standard output and standard error to files instead. <br />
用户命令最好不要带输出，否则输出会交错在一起，可以用重定向将命令的标准输出和错误输出重定向到文件。


### Special commands
* The special barrier synchronization command: `#sync`. <br />
  特殊的路障同步命令: `#sync` 。 <br>
  All threads will be suspended when `#sync` is executed, and waked up until the last thread executes the command. <br />
  所有线程执行到命令 `#sync` 时会等待，直到最后一个线程执行到此才被唤醒。 <br>
  Current version of `multirun` only support simple barrier synchronization, and other complicated synchronizations are not supported yet. <br />
  目前版本的 `multirun` 只提供了简单的同步路障功能，暂不支持更为复杂的指定命令依赖关系的操作。
* The special exiting command: `#exit`. <br />
  特殊的退出命令: `#exit` 。 <br />
  All commands after the exiting command will be ignored. <br />
  所有在退出命令之后的命令会被忽略。


Example 1: simple task
----------------------
* Goal: Using `gzip` to compress all text files (with extension `.txt`) in the `foo` directory. <br />
  目标: 用 `gzip` 压缩 `foo` 目录下面的纯文本文件(以`.txt`为扩展名)。
* Step 1: Generate input command file `input.cmd`, using following codes. <br />
  步骤 1: 生成输入命令文件 `input.cmd`, 可以用下面的命令生成。

        cd foo/ 
        ls *.txt | awk '{print "gzip " $0}' > ../input.cmd
        echo "#exit" >> ../input.cmd
        cd ../

* Show the input command file. <br />
  查看命令文件。

        gzip a.txt
        gzip b.txt
        ...
        gzip bar.txt
        #exit

* Step 2: Run `multirun` with 5 threads. <br />
  步骤 2: 运行 `multirun` 启动5个线程。

        /path/to/multirun input.cmd 5 -l log.txt

Example 2: task with synchronization
------------------------------------
* Goal: Downloading all webpages given URL list, then calculating TF-IDF. <br />
  目标: 下载一个URL列表中的所有网页，全部下载完了之后统计 TF-IDF。
* Existing programs. 已有程序: <br />
    `download_parse_one.sh`: download on page and store in directory `download`. <br />
    `count_tf_idf`: calculate TF-IDF of all files in given directory.
* Step 1: Generate input command file `input.cmd`, using following codes. <br />.
  步骤 1: 生成输入命令文件 `input.cmd`, 可以用下面的命令生成。

        awk '{print /path/to/download_parse_one.sh $0}' < url.txt > input.cmd
        echo "#sync" >> input.cmd
        echo "/path/to/count_tf_idf download/" >> input.cmd
        echo "#exit" >> input.cmd

* Show the input command file. <br />
  查看命令文件。

        /path/to/download_parse_one.sh http://www.baidu.com/
        ...
        /path/to/download_parse_one.sh http://www.google.com/
        #sync
        /path/to/count_tf_idf download/
        #exit

* Step 2: Run `multirun` with 10 threads. <br />
  步骤 2: 运行 `multirun` 启动10个线程。

        /path/to/multirun input.cmd 10 -l log.txt

multictrl
---------
Not implemented yet. <br />
尚未实现。

What's next?
------------
The next main version of multirun should support following operations: 

* 配置文件一定是可以直接用bash执行的，也就是说所有的附加控制信息都写在注释里

* 每个任务有一个唯一的可配置的字符串id, 在配置文件和动态调整的时候都用这个可读性比较好的id来代表任务

* 支持静态添加任务和设置任务拓扑结构

* 支持动态添加新任务并设置新任务和现有任务的拓扑关系

* 支持任务状态和拓扑结构查询功能(已完成, 执行中, 队列中, 未入队)

* 支持删除队列中和未入队任务

* 支持调整队列中任务执行顺序

* 可以动态修改线程的数目。

* 支持调整队列中和未入队任务的拓扑结构. 注意如果修改了队列中的任务的拓扑结构, 有可能会让任务从队列中移到未入队, 或相反

* 强制停止当前任务, 并自动删除依赖该任务的后续任务树

* 断点续行? 

* 不能修改任何已完成任务的状态

* 支持动态调整线程数目

* 核心结构是graph, 根据graph的拓扑排序来控制命令的顺序

* 每次并行执行入度为0的任务，当完成某个任务时，主线程判断修改入度，再把入度为0的添加到队列中。

