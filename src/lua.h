/*
** $Id: lua.h,v 1.218.1.5 2008/08/06 13:30:12 roberto Exp $
** Lua - An Extensible Extension Language
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"


#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS 	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"


/* mark for precompiled code (`<esc>Lua') */
//标记预编译代码的签名
//特殊标记并不是常规的ASCII字符，而是一个八进制转义序列
#define	LUA_SIGNATURE	"\033Lua"

/* option for multiple returns in `lua_pcall' and `lua_call' */
// 用于指定接收所有 Lua 函数返回值的标志
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/

//伪索引，用于访问特定索引的预定义位置或者计算相对索引的辅助宏

//这个宏定义了索引 -10000，用于访问 Lua 状态机的注册表（registry）。
//注册表是一个全局的 Lua 表，用于存储一些全局共享的数据和对象。
#define LUA_REGISTRYINDEX	(-10000)

//这个宏定义了索引 -10001，用于访问函数的环境表（environment table）。
//在 Lua 中，每个函数都可以有一个关联的环境表，用于存储该函数的上值upvalue以及其他相关信息。
#define LUA_ENVIRONINDEX	(-10001)

//这个宏定义了索引 -10002，用于访问全局环境表（_G）。
//全局环境表存储了 Lua 程序的全局变量，也可以用于操作全局环境中的其他对象。
#define LUA_GLOBALSINDEX	(-10002)

//这个宏是一个计算相对索引的辅助宏，用于访问函数的上值（upvalue）。
//上值是在闭包中被捕获的外部变量，通过这个宏可以访问闭包的上值。
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status; 0 is OK */
//表示线程（协程）的不同状态或错误类型
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


//为 lua_State 结构体类型创建一个别名，以便在代码中使用这个别名来声明指向 Lua 状态机的指针，同时也在头文件中隐藏了实际结构体的细节
typedef struct lua_State lua_State;

typedef int (*lua_CFunction) (lua_State *L);


/**
 * @brief 加载/转储Lua块时读取/写入块的函数
 */
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);


/**
 * @brief 内存分配函数原型
 */
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** basic types
*/
//基础类型
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8



/* minimum Lua stack available to a C function */
//当你在 Lua 中创建一个新的状态机时，它会分配一个初始容量为 LUA_MINSTACK 的栈
#define LUA_MINSTACK	20


/*
** generic extra include file
*/
//通用额外包含文件
//用户可以通过定义 LUA_USER_H 宏并提供相应的头文件来扩展 Lua 的功能。
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


//在编写 Lua 代码时，你可以使用 lua_Number 和 lua_Integer 来表示数字和整数，而不需要关心具体的底层实现。这有助于提高代码的可移植性和可维护性
/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;

/* type for integer functions */
typedef LUA_INTEGER lua_Integer;



/*
 * *状态操作函数
*/


/**
 * @brief 创建新状态机
 * @param f 内存分配函数
 * @param ud 用户自定义数据
 * @return lua_State* 
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);

/**
 * @brief 关闭一个 Lua 状态机（解释器实例），释放相关的内存资源
 * @param L 需要关闭的lua状态机
 * @return 无返回值
 */
LUA_API void       (lua_close) (lua_State *L);

/**
 * @brief 从状态机中创建新线程（协程）并返回其指针
 * @param L 指定的状态机
 * @return 状态机指针
 */
LUA_API lua_State *(lua_newthread) (lua_State *L);

/**
 * @brief 设置一个自定义的 panic 处理函数，在发生致命错误时，Lua 解释器会调用这个函数，以进行自定义的错误处理
 * @param L 指定的状态机
 * @param panicf 指定的错误处理操作
 * @return 返回原来的错误处理函数
 */
LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


/**
 * 基本堆栈操作
*/

/**
 * @brief 获取 Lua 状态机栈顶的索引（也就是栈上的元素数量）
 * @param L 指定的状态机
 * @return 返回当前栈上元素的数量
 */
LUA_API int   (lua_gettop) (lua_State *L);

/**
 * @brief 设置 Lua 栈的栈顶位置，以便在栈上进行元素的添加、删除或者调整。你可以使用这个函数来调整栈的状态，确保栈上的元素数量满足你的需求
 * @param L 指定的状态机
 * @param idx 整数参数，用于指定新的栈顶位置。栈的栈顶位置是指栈顶元素的索引，如果 idx 大于当前栈上的元素数量，将会压入空值（nil）来扩展栈；如果 idx 小于当前栈上的元素数量，将会弹出栈上多余的元素，将栈的栈顶位置移动到指定位置。
 * @return 
 */
LUA_API void  (lua_settop) (lua_State *L, int idx);

/**
 * @brief 复制栈上指定索引位置的值并将复制的值推入栈顶
 * @param L 指定的状态机
 * @param idx 指定要复制的值的索引位置。函数将栈上的这个位置的值复制一份，并将复制的值推入栈顶
 * @return 
 */
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);

/**
 * @brief 从栈中移除位于指定索引位置 index 的元素
 * @param L 指定的状态机
 * @param idx 指定要移除的位于指定索引位置 idx 的元素。该索引位置之上的元素会依次下移填补空缺
 * @return 
 */
LUA_API void  (lua_remove) (lua_State *L, int idx);

/**
 * @brief 在栈中的指定索引位置 index 处插入一个新的元素
 * @param L 指定的状态机
 * @param idx 指定新的元素的索引位置
 * @return 
 */
LUA_API void  (lua_insert) (lua_State *L, int idx);

/**
 * @brief 用栈上指定索引位置的元素替换为栈顶的元素，同时去掉栈顶元素
 * @param L 
 * @param idx 指定替换元素的索引位置
 * @return 
 */
LUA_API void  (lua_replace) (lua_State *L, int idx);

/**
 * @brief 确保栈上有足够的空闲槽位来进行操作，可以用于预先分配空间以避免栈溢出
 * @param L 
 * @param sz 栈上的空闲槽位不足 extra 个，它会自动扩展栈的大小以满足需求
 * @return 返回值是否成功扩展栈。如果成功，返回 1；否则返回 0
 */
LUA_API int   (lua_checkstack) (lua_State *L, int sz);

/**
 * @brief 在不同的 Lua 状态机之间传递数据，它是在多线程或嵌套 Lua 环境中进行数据交互的重要工具
 * @param from 源状态机
 * @param to 目标状态机
 * @param n 从源状态机的栈顶开始移动 n 个元素到目标状态机的栈顶，然后在源状态机上删除这些元素
 * @return 
 */
LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/
LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/
LUA_API void  (lua_pushnil) (lua_State *L);
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API void  (lua_gettable) (lua_State *L, int idx);
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawget) (lua_State *L, int idx);
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API void  (lua_getfenv) (lua_State *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setfenv) (lua_State *L, int idx);


/*
** `load' and `call' functions (load and run Lua code)
*/
LUA_API void  (lua_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int   (lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                                        const char *chunkname);

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data);


/*
** coroutine functions
*/
LUA_API int  (lua_yield) (lua_State *L, int nresults);
LUA_API int  (lua_resume) (lua_State *L, int narg);
LUA_API int  (lua_status) (lua_State *L);

/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
// 整体的垃圾收集,一直到全部回收
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
// 进行单次的垃圾收集
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7

LUA_API int (lua_gc) (lua_State *L, int what, int data);


/*
** miscellaneous functions
*/

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);



/* 
** ===============================================================
** some useful macros
** ===============================================================
*/

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)



/*
** compatibility macros and functions
*/

#define lua_open()	luaL_newstate()

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)

#define lua_getgccount(L)	lua_gc(L, LUA_GCCOUNT, 0)

#define lua_Chunkreader		lua_Reader
#define lua_Chunkwriter		lua_Writer


/* hack */
LUA_API void lua_setlevel	(lua_State *from, lua_State *to);


/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record */


/* Functions to be called by the debuger in specific events */
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);

LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook lua_gethook (lua_State *L);
LUA_API int lua_gethookmask (lua_State *L);
LUA_API int lua_gethookcount (lua_State *L);


struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) `global', `local', `field', `method' */
  const char *what;	/* (S) `Lua', `C', `main', `tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
