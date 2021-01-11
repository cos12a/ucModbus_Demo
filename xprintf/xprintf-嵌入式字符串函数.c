xprintf-嵌入式字符串函数
xprintf是一个紧凑的字符串I / O库。对于程序存储器不足以实现常规printf功能的微型微控制器而言，它是理想的选择。推荐的用法是：将格式化的字符串写入LCD或UART，以及用于调试/维护控制台。

可以使用配置选项配置xprintf以减小模块大小。下表显示了Cortex-M3中的代码大小示例（gcc -Os）。

特征	。文本
基本输出	738
长长整数	+110
浮点	+849
输入值	+286
应用介面
嵌入式字符串函数提供以下函数。

输出量
/ * ---------------------------------------------- /
/ xputc-放置一个字符
/ ---------------------------------------------- * /

void xputc（
    int chr            / *要输出的字符（0-255）* /
）;

void xfputc（
    void（* func）（int），/ *指向输出函数的指针* / 
    int chr            / *要输出的字符（0-255）* /
）;
/ * ---------------------------------------------- /
/ xputs-放置以空值结尾的字符串
/ ---------------------------------------------- * /

void xputs（
    const char * str    / *指向要输出的空终止字符串的指针* /
）;

void xfputs（
    void（* func）（int），/ *指向输出函数的指针* / 
    const char * str    / *指向要输出的以空值终止的字符串的指针* /
）;
/ * ---------------------------------------------- /
/ xprintf-格式化的字符串输出
/ ---------------------------------------------- * /

void xprintf（         / *将格式化的字符串放入默认设备* / 
    const char * fmt，   / *指向以空字符终止的格式字符串* / 
    ...                / *可选参数... * /
）;

void xfprintf（        / *将格式化的字符串放入指定的设备* / 
    void（* func）（int），/ *指向输出函数的指针* / 
    const char * fmt，   / *指向以空值终止的格式字符串* / 
    ...                / *可选参数... * /
）;

void xsprintf（        / *将格式化的字符串放入内存* / 
    char * buff，        / *指向缓冲区的指针以存储输出字符串* / 
    const char * fmt，   / *指向以null终止的格式字符串* / 
    ...                / *可选参数... * /
）;
格式控制指令是标准库的子集，如下所示：

    ％[flag] [width] [。precision] [size] type
旗
填充选项。一个-左对齐指定。甲0指定零填充。默认设置为右对齐并填充空格。
宽度
字段的最小宽度，为1-99或*。如果生成的字符串的宽度小于指定的值，则用空格或零填充rest字段。一个*指定的值来自于整型参数。
。精确
指定小数位数或字符串的最大宽度，.0-.99或。*。数字的默认设置为6，字符串的默认设置为无限制。
尺寸
指定整数参数l（长）和ll（长长）的大小。如果sizeof（long）== sizeof（int）为true（这是32位系统的典型值），则长整数参数可以省略前缀l。对于整数参数，默认大小为int，并且始终假定浮点参数为double。
类型
csduoxbfe指定参数的类型，字符，字符串，十进制无符号整数，十进制无符号整数，八进制无符号整数，十六进制无符号整数，二进制无符号整数，十进制表示法的浮点数和E表示法的浮点数。X和E以大写形式生成数字。
例子：
    xprintf（“％d”，1234）;             / *“ 1234” * / 
    xprintf（“％6d，％3d %%”，-200，5）;   / *“    -200，5 ％” * / 
    xprintf（“％-6u”，100）;            / *“ 100” * / 
    xprintf（“％ld”，12345678）;        / *“ 12345678” * / 
    xprintf（“％llu”，0x100000000）;    / *“ 4294967296” <XF_USE_LLI> * / 
    xprintf（“％lld”，-1LL）;           / *“ -1” <XF_USE_LLI> * / 
    xprintf（“％04x”，0xA3）;           / *“ 00a3” * / 
    xprintf（“％08lX”，0x123ABC）;      / *“ 00123ABC” * / 
    xprintf（“％016b”，        0x550F）; / *“ 0101010100001111” * / 
    xprintf（“％* d”，6，100）;          / *“ 100” * /
    xprintf（“％s”，“ String”）;         / *“ String” * / 
    xprintf（“％5s”，“ abc”）;           / *“ abc” * / 
    xprintf（“％5.5s”，“ abcdefg”）;     / *“ abcde” * / 
    xprintf（“％-5s”，“ abc”）;          / *“ abc” * / 
    xprintf（“％-5s”，“ abcdefg”）;      / *“ abcdefg” * / 
    xprintf（“％-5.5s”，“ abcdefg”）;    / *“ abcde” * / 
    xprintf（“％c”，'a'）;              / *“ a” * / 
    xprintf（“％12f”，10.0）;           / *“ 10.000000” <XF_USE_FP> 
    * / xprintf（“％。4E”，123.45678）;      / *“ 1.2346E + 02” <XF_USE_FP> * /
/ * ---------------------------------------------- /
/ put_dump-放入一行二进制转储
/ ---------------------------------------------- * /

void put_dump（
    const void * buff，    / *指向要显示的数据的指针* / 
    unsigned long adr，   / *标题地址* / 
    int cnt，             / *要显示的项目数* / 
    int width           / *项目大小（1、2或4）* /
）;
输入值
/ * ---------------------------------------------- /
/ xgets-从输入设备获取一行
/ ---------------------------------------------- * /

int xgets（      / * 0：流结束，1：一行到达* / 
    char * buff，/ *指向要输入的缓冲区的指针* / 
    int len      / *缓冲区长度* /
）;
/ * ---------------------------------------------- /
/ xatoi-获取整数字符串的值
/ ---------------------------------------------- * /
/ *“ 123 -5 0x3ff 0b1111 0377 1.5”
       ^第一个调用返回123和下一个ptr
          ^第二次通话返回-5，然后返回下一个点
                   ^第三次调用返回1023和下一个ptr
                          ^第四次调用返回15和下一个ptr
                               ^第5次通话返回255，然后返回下一个点
                                 ^第6次呼叫失败并返回0
* /

int xatoi（       / * 0：失败，1：成功* / 
    char ** str，   / *指向字符串的指针* / 
    long * res     / *指向存储值的变量的指针* /
）;
/ * ---------------------------------------------- /
/ xatof-获取浮点字符串的值
/ ---------------------------------------------- * /
/ *“ 123 -5.75 .6 + 8.88E + 5 1e-6。”
       ^第一个通话返回1.23e2和下一个ptr
             ^第二次调用返回-5.75e0和下一个ptr
                ^第三次调用返回6e-1和下一个ptr
                           ^第四次调用返回8.88e5和下一个ptr
                                ^第5次通话返回1e-6和下一个ptr
                                   ^第6次呼叫失败并返回0
* /

int xatof（       / * 0：失败，1：成功* / 
    char ** str，   / *指向字符串的指针* / 
    double * res   / *指向存储值的变量的指针* /
）;
设备I / O功能
输出功能是用户提供的回叫功能，用于将字节写入输出设备。其地址应设置为模块（默认输出设备）中的功能指针xfunc_output。通常，此功能将字节放入UART，LCD或某些输出设备。输出函数从xputc（）回调。有一个宏可以简化设置。例如，当附加void uart1_putc（uint8_t chr）时；到模块xdev_out（uart1_putc）; 会做。如果输出函数具有多个参数，或者简单的输出函数不可用，则需要粘合功能。xfputc（），xfputs（），xfprintf（）和xsprintf（） 用其参数覆盖默认输出设备。

输入功能是用户提供的回叫功能，用于从输入设备读取字节。其地址应设置为函数指针xfunc_input，默认输入设备。有一个宏xdev_in（）可以简化设置。例如xdev_in（uart1_getc）; 所述xfgets（）函数覆盖使用其参数的默认输入设备。输入函数从xgets（）函数中回调。通常，输入函数从输入设备或文件中读取一个字节。当输入设备报告流结束时，输入函数应返回-1。该xgets（）与零和应用函数终止将能够探测到它。

/ *向输出设备写入一个字节* /

void output_func（
    int chr   / *要写入的字节* /
）;
/ *从输入设备读取一个字节* /

int input_func（void）;  / *作为读取字符返回0到255，作为EOF返回-1 * /



/*----------------------------------------------/
/  xgets - Get a line from the input device
/----------------------------------------------*/

int xgets (     /* 0:End of stream, 1:A line arrived */
    char* buff, /* Pointer to the buffer to input */
    int len     /* Buffer length */
);
/*----------------------------------------------/
/  xatoi - Get a value of integer string
/----------------------------------------------*/
/* "123 -5    0x3ff 0b1111 0377 1.5 "
       ^                            1st call returns 123 and next ptr
          ^                         2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                 ^  6th call fails and returns 0
*/

int xatoi (      /* 0:Failed, 1:Succeeded */
    char** str,  /* Pointer to pointer to the string */
    long* res    /* Pointer to the valiable to store the value */
);
/*----------------------------------------------/
/  xatof - Get a value of floating point string
/----------------------------------------------*/
/* "123 -5.75 .6   +8.88E+5 1e-6  .  "
       ^                              1st call returns 1.23e2 and next ptr
             ^                        2nd call returns -5.75e0 and next ptr
                ^                     3rd call returns 6e-1 and next ptr
                           ^          4th call returns 8.88e5 and next ptr
                                ^     5th call returns 1e-6 and next ptr
                                   ^  6th call fails and returns 0
*/

int xatof (      /* 0:Failed, 1:Succeded */
    char** str,  /* Pointer to pointer to the string */
    double* res  /* Pointer to the valiable to store the value */
);