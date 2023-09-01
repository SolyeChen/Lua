/*
** $Id: lstate.h,v 2.24.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#include "lua.h"

#include "lobject.h"
#include "ltm.h"
#include "lzio.h"



struct lua_longjmp;  /* defined in ldo.c */


/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)



typedef struct stringtable {
  GCObject **hash;
  lu_int32 nuse;  /* number of elements */
  int size;       // hash桶数组大小
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */
  StkId func;  /* function index in the stack */
  StkId	top;  /* top for this function */
  const Instruction *savedpc;
  int nresults;  /* expected number of results from this function */
  int tailcalls;  /* number of tail calls lost under this entry */
} CallInfo;



#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->c.isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;
  lu_byte gcstate;  /* state of garbage collector */
  int sweepstrgc;  /* position of sweep in `strt' */
  GCObject *rootgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* position of sweep in `rootgc' */
  GCObject *gray;  /* list of gray objects */
  GCObject *grayagain;  /* list of objects to be traversed atomically */
  GCObject *weak;  /* list of weak tables (to be cleared) */
  // 所有有GC方法的udata都放在tmudata链表中
  GCObject *tmudata;  /* last element of list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  // 一个阈值，当这个totalbytes大于这个阈值时进行自动GC
  lu_mem GCthreshold;
  // 保存当前分配的总内存数量
  lu_mem totalbytes;  /* number of bytes currently allocated */
  // 一个估算值，根据这个计算GCthreshold
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  // 当前待GC的数据大小，其实就是累加totalbytes和GCthreshold的差值
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  // 可以配置的一个值，不是计算出来的，根据这个计算GCthreshold，以此来控制下一次GC触发的时间
  int gcpause;  /* size of pause between successive GCs */
  // 每次进行GC操作回收的数据比例，见lgc.c/luaC_step函数
  int gcstepmul;  /* GC `granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue l_registry;
  struct lua_State *mainthread;
  UpVal uvhead;  /* head of double-linked list of all open upvalues */
  struct Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */
} global_State;


/**
 * *每个线程的状态
 * Lua的运行环境
*/
struct lua_State {
  CommonHeader;
  lu_byte status;/* 用于表示线程状态，可能是 LUA_OK、LUA_YIELD、LUA_ERRRUN 等之一 */
  StkId top;  /* 指向当前栈顶元素的指针。在栈上进行操作时，这个指针会随着元素的入栈和出栈而改变。Lua 解释器通过这个指针来管理栈上的元素 */
  StkId base;  /* 指向当前函数调用的第一个参数的位置。当调用一个 Lua 函数时，它的参数会被放在栈上，base 指示了参数的位置。函数执行完后，这个位置也会被用于存放函数的返回值 */
  global_State *l_G;/* 全局状态信息的指针，包含了一些全局的配置和状态信息 */
  CallInfo *ci;  /* 调用帧（Call Info）结构体指针，用于表示函数调用的信息。每次调用一个函数，都会创建一个新的调用帧并将其链接起来，形成一个调用链*/
  const Instruction *savedpc;  /* `savedpc' of current function */
  StkId stack_last;  /* 指向栈中最后一个空闲槽（slot）的指针。它通常用于确定新值应该存储在哪个槽位上 */
  StkId stack;  /* 指向整个 Lua 栈的起始位置的指针 */
  CallInfo *end_ci;  /* ci数组结束后的点*/
  CallInfo *base_ci;  /* ci数组 */
  int stacksize;
  int size_ci;  /* ci数组大小' */
  unsigned short nCcalls;  /* 嵌套的C调用数 */
  unsigned short baseCcalls;  /* 恢复协同程序时的嵌套C调用 */
  lu_byte hookmask; /* 控制钩子函数何时执行的标志 */
  lu_byte allowhook; /* 标识当前是否允许执行钩子函数。如果为非零值，表示允许执行钩子函数；如果为零值，表示不允许执行钩子函数 */
  int basehookcount; /* 存储计数开始时的值，当 hookcount 达到这个值时，会执行钩子函数 */
  int hookcount; /* 控制何时触发钩子函数的计数  */
  lua_Hook hook; /*用于设置钩子函数，监控代码执行*/
  TValue l_gt;  /* 指向全局环境表，允许 Lua 解释器在全局环境中查找和修改全局变量 */
  TValue env;  /* 存储临时的环境， 在 Lua 中，环境可以用于限定变量的作用范围。这个字段通常用于临时存储函数的环境，例如，当函数闭包（closure）创建时，它的环境可能会存储在这个字段中。*/
  GCObject *openupval;  /* 指向 GCObject 结构体的指针，用于表示在当前栈上的开放的上值（upvalues）的链表。上值是闭包中引用的外部局部变量，它们可以跨越不同的作用域。这个字段用于管理在当前栈帧上的开放的上值 */
  GCObject *gclist; /* 指向 GCObject 结构体的指针，用于表示垃圾回收链表 */
  struct lua_longjmp *errorJmp;  /* 当前的错误恢复点。在 Lua 中，当发生错误时，可以使用长跳转（longjmp）来快速跳出多层函数调用并执行错误处理。errorJmp 用于存储当前的错误恢复点。 */
  ptrdiff_t errfunc;  /* 当前的错误处理函数在栈上的索引。当发生错误时，Lua 可以调用一个用户指定的错误处理函数来处理错误。errfunc 用于指示当前使用的错误处理函数在栈上的位置。 */
};


/**
 * 可以通过 G(L) 来访问 Lua 状态机的全局状态信息
*/
#define G(L)	(L->l_G)


/**
 * @brief 所有可收集（collectable）对象的共同结构
 */
union GCObject {
  GCheader gch;         //GC头对象
  union TString ts;     //字符串对象
  union Udata u;        //用户数据（userdata）对象
  union Closure cl;     //闭包对象
  struct Table h;       //表对象
  struct Proto p;       //函数原型（prototype）对象
  struct UpVal uv;      //上值（upvalue）对象
  struct lua_State th;  //线程（thread）对象
};


/* 用于将GCObject转换为特定值的宏 */
#define rawgco2ts(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gco2ts(o)	(&rawgco2ts(o)->tsv)
#define rawgco2u(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gco2u(o)	(&rawgco2u(o)->uv)
#define gco2cl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
#define gco2h(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
#define gco2p(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gco2uv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define gco2th(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* 将任何Lua对象转换为GCObject的宏 */
#define obj2gco(v)	(cast(GCObject *, (v)))


LUAI_FUNC lua_State *luaE_newthread (lua_State *L);
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);

#endif
