# faketransfer-multithread
a file transfer only with upload function

## Technology
* TCP
* **epoll**
* **threadpool**
* **mmap**

## Test Environment
* Two CentOS7.2 on the same host by VMware Workstation 12 Player
* 1 core i5-6500 CPU @ 3.20GHz and 1GB memory for each test VMware machine
* 7200 rpm hard disk
* transfer a 3GB file

## Operation
### vm-one-server
* `cd faketransfer/faketransfer-multithread`
* `make`
* `./faketransfer-mt-svr`

### vm-two-client
* `cd faketransfer/faketransfer-multithread`
* `make`
* `./faketransfer-mt-cli`
* input the file name with absolute path

## Result-mt
* Time cost between **38s ~ 60s**
