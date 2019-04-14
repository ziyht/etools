# etools 0.9


  目前仅支持 Windows/Linux

  个人用 C 基础工具包


# 工具列表

管理工具：

    emake   - 工程管理框架


集成外部库：

    libuv   - 跨平台异步事件框架
    cnats   - NATS平台基础库


基本容器：

    ejson   - json解析器 （重构于 cjson）
    ell     - 链表
    erb     - 红黑树     （重构于 linux kernel）
    esl     - 跳表       （重构于 redis skiplist）
    edict   - 字典       （重构于 redis dict）


字符串：

    estr    - 动态字串（重构于 redis sds）


多线程：

    ethread - 线程
                linux    基于 pthread
                windows  基于 pthread_on_win32
    etp     - 线程池
    echan   - 通道： 用于线程间通信，类似于 go 的 chan


其他

    etimer  - 定时器
    elog    - 日志系统
    earg    - 命令行参数解析器


待实现：

    evar  - 可变容器
    evec  - 向量
    eerr  - 错误
    eco   - 协程
    eatom - 原子锁


# ejson

特性：

    1. 支持注释，/**/ // #，（为了方便用json当配置文件）
    2. 数组使用双链表维护
    3. obj 同时使用双链表和 dict（redis） 维护
    4. 添加了 PTR 和 RAW 属性，在必要的情况下，方便管理自定义数据
        （格式化时暂时只简单写入字符串式的头信息）
    5. 支持解析式的key查询
    6. 添加排序功能（重构后暂时移除）


ejson 在一定程度上可以当成是 edict 和 ell 的集成版：

    1. 保留了 obj 中 item 的插入顺序
    2. 对obj的查询具有极高的效率
    3. 可以使用链表的方式遍历，遍历时可以不考虑内部字典的rehash操作
    4. 占用更多的空间(呃...)


# ell


特性：

    1. 内部自动记录上次查询的节点，随机查询比普通链表提升一倍性能（貌似没啥）
    2. 可以使用 for(int i = 0; i < ell_len(l); i++) 的方式遍历链表，但是是非线程安全的


# erb

# edict

# esl
