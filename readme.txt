在wsl2(ubuntu20.04) 上安装了MySql8.4.6版本，使用SQLAPI++Library 5.4.0 作为客户端连接库。 


db_info/mysql_table_def.sql 中是我们项目需要的一些表结构；
SQLAPI++Library 解压后的头文件在include/sqlapi，动态库在library;


c++ 使用的是c++17标准,从mysql 中读取相关的表，并且记录在程序的缓存中。

我的需求： 
1. 帮我实现每个表创建对应的结构体，并且把数据读出来的功能。
2. 模型表T_DD_SM_MDL 和 模型属性表T_DD_SM_MDL_PROP。 
模型属性表中是每个模型有那些成员变量，成员变量的类型是T_DD_SM_TYPE 定义的，可能是基本类型，也可能是数组、子结构体。
type_extra 字段内容说明：
如果是基本类型则不需要，为空
如果是STRING 类型：{"RANGE": [], "ELE_TYPE": "", "MODEL_ID": 0, "STR_LENGTH": 30};
如果是模型：{"RANGE": [], "ELE_TYPE": "", "MODEL_ID": 1, "STR_LENGTH": 0};
如果是字符串数组（每个字符串是30）：{"RANGE": [{"end": 2, "start": 0}], "ELE_TYPE": "STRING", "MODEL_ID": 0, "STR_LENGTH": 30};
如果是模型数组 {"RANGE": [{"end": 2, "start": 0}], "ELE_TYPE": "STD::UDT1", "MODEL_ID": 1, "STR_LENGTH": 0};
如果是基础类型 {"RANGE": [{"end": 2, "start": 0}], "ELE_TYPE": "INT", "MODEL_ID": 0, "STR_LENGTH": 0}。

模型表中定义了他们的对齐方式。 
我想计算拓展信息，比如模型属性表中的每个成员变量起始偏移和长度，模型的总长度。
3. 模型散点表 T_DD_SM_MDL_INFO 
定义了模型的所有叶子成员，包括基本成员，数组成员中的每个元素， 子结构体成员中的每个成员，比如：
STR2
U1V1.DINT1
U1V1.ARR_STR1[2]
U1V1.ARR_STR2[1]
U1V1.ARR_U1V1[0].DINT1
U1V1.ARR_U1V1[2].LINT1
我想得到每个叶子成员的起始偏移和长度。

4.  变量表 T_DD_SM_VAR  和  变量基础表 T_DD_SM_VAR_PROP；
如果变量表中是基本类型，则在 变量基础表中也只有一条记录。 
如果是数组，则每个数组元素在变量基础表中都有记录。
如果是模型，则模型中的每个成员变量在变量基础表中都有记录。
总之变量基础表中是叶子成员变量,距离数据见 db_info/T_DD_SM_VAR.json  db_info/T_DD_SM_VAR_PROP.json

我想得到变量基础表中每个变量的起始偏移和长度。

 