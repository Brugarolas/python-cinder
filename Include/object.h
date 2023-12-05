#ifndef Py_OBJECT_H
#define Py_OBJECT_H

#include "cinder/ci_api.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Object and type object interface */

/*
Objects are structures allocated on the heap.  Special rules apply to
the use of objects to ensure they are properly garbage-collected.
Objects are never allocated statically or on the stack; they must be
accessed through special macros and functions only.  (Type objects are
exceptions to the first rule; the standard types are represented by
statically initialized type objects, although work on type/class unification
for Python 2.2 made it possible to have heap-allocated type objects too).

An object has a 'reference count' that is increased or decreased when a
pointer to the object is copied or deleted; when the reference count
reaches zero there are no references to the object left and it can be
removed from the heap.

An object has a 'type' that determines what it represents and what kind
of data it contains.  An object's type is fixed when it is created.
Types themselves are represented as objects; an object contains a
pointer to the corresponding type object.  The type itself has a type
pointer pointing to the object representing the type 'type', which
contains a pointer to itself!.

Objects do not float around in memory; once allocated an object keeps
the same size and address.  Objects that must hold variable-size data
can contain pointers to variable-size parts of the object.  Not all
objects of the same type have the same size; but the size cannot change
after allocation.  (These restrictions are made so a reference to an
object can be simply a pointer -- moving an object would require
updating all the pointers, and changing an object's size would require
moving it if there was another object right next to it.)

Objects are always accessed through pointers of the type 'PyObject *'.
The type 'PyObject' is a structure that only contains the reference count
and the type pointer.  The actual memory allocated for an object
contains other data that can only be accessed after casting the pointer
to a pointer to a longer structure type.  This longer type must start
with the reference count and type fields; the macro PyObject_HEAD should be
used for this (to accommodate for future changes).  The implementation
of a particular object type can cast the object pointer to the proper
type and back.

A standard interface exists for objects that contain an array of items
whose size is determined when the object is allocated.
*/

/* Py_DEBUG implies Py_REF_DEBUG. */
#if defined(Py_DEBUG) && !defined(Py_REF_DEBUG)
#  define Py_REF_DEBUG
#endif

#if defined(Py_LIMITED_API) && defined(Py_TRACE_REFS)
#  error Py_LIMITED_API is incompatible with Py_TRACE_REFS
#endif

/* PyTypeObject structure is defined in cpython/object.h.
   In Py_LIMITED_API, PyTypeObject is an opaque structure. */
typedef struct _typeobject PyTypeObject;

#ifdef Py_TRACE_REFS
/* Define pointers to support a doubly-linked list of all live heap objects. */
#define _PyObject_HEAD_EXTRA            \
    struct _object *_ob_next;           \
    struct _object *_ob_prev;

#define _PyObject_EXTRA_INIT 0, 0,

#else
#  define _PyObject_HEAD_EXTRA
#  define _PyObject_EXTRA_INIT
#endif

/* PyObject_HEAD defines the initial segment of every PyObject. */
#define PyObject_HEAD                   PyObject ob_base;

#define PyObject_HEAD_INIT(type)        \
    { _PyObject_EXTRA_INIT              \
    1, type },

#define PyObject_HEAD_IMMORTAL_INIT(type)        \
    { _PyObject_EXTRA_INIT kImmortalInitialCount, type },

#define PyVarObject_HEAD_INIT(type, size)       \
    { PyObject_HEAD_IMMORTAL_INIT(type) size },

/* PyObject_VAR_HEAD defines the initial segment of all variable-size
 * container objects.  These end with a declaration of an array with 1
 * element, but enough space is malloc'ed so that the array actually
 * has room for ob_size elements.  Note that ob_size is an element count,
 * not necessarily a byte count.
 */
#define PyObject_VAR_HEAD      PyVarObject ob_base;
#define Py_INVALID_SIZE (Py_ssize_t)-1

/* Nothing is actually declared to be a PyObject, but every pointer to
 * a Python object can be cast to a PyObject*.  This is inheritance built
 * by hand.  Similarly every pointer to a variable-size Python object can,
 * in addition, be cast to PyVarObject*.
 */
typedef struct _object {
    _PyObject_HEAD_EXTRA
    Py_ssize_t ob_refcnt;
    PyTypeObject *ob_type;
} PyObject;

/* Cast argument to PyObject* type. */
#define _PyObject_CAST(op) ((PyObject*)(op))
#define _PyObject_CAST_CONST(op) ((const PyObject*)(op))

typedef struct {
    PyObject ob_base;
    Py_ssize_t ob_size; /* Number of items in variable part */
} PyVarObject;

/* Cast argument to PyVarObject* type. */
#define _PyVarObject_CAST(op) ((PyVarObject*)(op))
#define _PyVarObject_CAST_CONST(op) ((const PyVarObject*)(op))


// Test if the 'x' object is the 'y' object, the same as "x is y" in Python.
PyAPI_FUNC(int) Py_Is(PyObject *x, PyObject *y);
#define Py_Is(x, y) ((x) == (y))


static inline Py_ssize_t _Py_REFCNT(const PyObject *ob) {
    return ob->ob_refcnt;
}
#define Py_REFCNT(ob) _Py_REFCNT(_PyObject_CAST_CONST(ob))


// bpo-39573: The Py_SET_TYPE() function must be used to set an object type.
#define Py_TYPE(ob)             (_PyObject_CAST(ob)->ob_type)

// bpo-39573: The Py_SET_SIZE() function must be used to set an object size.
#define Py_SIZE(ob)             (_PyVarObject_CAST(ob)->ob_size)


static inline int _Py_IS_TYPE(const PyObject *ob, const PyTypeObject *type) {
    // bpo-44378: Don't use Py_TYPE() since Py_TYPE() requires a non-const
    // object.
    return ob->ob_type == type;
}
#define Py_IS_TYPE(ob, type) _Py_IS_TYPE(_PyObject_CAST_CONST(ob), type)


static inline void _Py_SET_TYPE(PyObject *ob, PyTypeObject *type) {
    ob->ob_type = type;
}
#define Py_SET_TYPE(ob, type) _Py_SET_TYPE(_PyObject_CAST(ob), type)


static inline void _Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
    ob->ob_size = size;
}
#define Py_SET_SIZE(ob, size) _Py_SET_SIZE(_PyVarObject_CAST(ob), size)


/*
Type objects contain a string containing the type name (to help somewhat
in debugging), the allocation parameters (see PyObject_New() and
PyObject_NewVar()),
and methods for accessing objects of the type.  Methods are optional, a
nil pointer meaning that particular kind of access is not available for
this type.  The Py_DECREF() macro uses the tp_dealloc method without
checking for a nil pointer; it should always be implemented except if
the implementation can guarantee that the reference count will never
reach zero (e.g., for statically allocated type objects).

NB: the methods for certain type groups are now contained in separate
method blocks.
*/

typedef PyObject * (*unaryfunc)(PyObject *);
typedef PyObject * (*binaryfunc)(PyObject *, PyObject *);
typedef PyObject * (*ternaryfunc)(PyObject *, PyObject *, PyObject *);
typedef int (*inquiry)(PyObject *);
typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef PyObject *(*ssizessizeargfunc)(PyObject *, Py_ssize_t, Py_ssize_t);
typedef int(*ssizeobjargproc)(PyObject *, Py_ssize_t, PyObject *);
typedef int(*ssizessizeobjargproc)(PyObject *, Py_ssize_t, Py_ssize_t, PyObject *);
typedef int(*objobjargproc)(PyObject *, PyObject *, PyObject *);

typedef int (*objobjproc)(PyObject *, PyObject *);
typedef int (*visitproc)(PyObject *, void *);
typedef int (*traverseproc)(PyObject *, visitproc, void *);


typedef void (*freefunc)(void *);
typedef void (*destructor)(PyObject *);
typedef PyObject *(*getattrfunc)(PyObject *, char *);
typedef PyObject *(*getattrofunc)(PyObject *, PyObject *);
typedef int (*setattrfunc)(PyObject *, char *, PyObject *);
typedef int (*setattrofunc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*reprfunc)(PyObject *);
typedef Py_hash_t (*hashfunc)(PyObject *);
typedef PyObject *(*richcmpfunc) (PyObject *, PyObject *, int);
typedef PyObject *(*getiterfunc) (PyObject *);
typedef PyObject *(*iternextfunc) (PyObject *);
typedef PyObject *(*descrgetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*descrsetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*initproc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*newfunc)(PyTypeObject *, PyObject *, PyObject *);
typedef PyObject *(*allocfunc)(PyTypeObject *, Py_ssize_t);

typedef struct{
    int slot;    /* slot id, see below */
    void *pfunc; /* function pointer */
} PyType_Slot;

typedef struct{
    const char* name;
    int basicsize;
    int itemsize;
    unsigned int flags;
    PyType_Slot *slots; /* terminated by slot==0. */
} PyType_Spec;

PyAPI_FUNC(PyObject*) PyType_FromSpec(PyType_Spec*);
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03030000
PyAPI_FUNC(PyObject*) PyType_FromSpecWithBases(PyType_Spec*, PyObject*);
#endif
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03040000
PyAPI_FUNC(void*) PyType_GetSlot(PyTypeObject*, int);
#endif
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03090000
PyAPI_FUNC(PyObject*) PyType_FromModuleAndSpec(PyObject *, PyType_Spec *, PyObject *);
PyAPI_FUNC(PyObject *) PyType_GetModule(struct _typeobject *);
PyAPI_FUNC(void *) PyType_GetModuleState(struct _typeobject *);
#endif

/* Generic type check */
PyAPI_FUNC(int) PyType_IsSubtype(PyTypeObject *, PyTypeObject *);

static inline int _PyObject_TypeCheck(PyObject *ob, PyTypeObject *type) {
    return Py_IS_TYPE(ob, type) || PyType_IsSubtype(Py_TYPE(ob), type);
}
#define PyObject_TypeCheck(ob, type) _PyObject_TypeCheck(_PyObject_CAST(ob), type)

PyAPI_DATA(PyTypeObject) PyType_Type; /* built-in 'type' */
PyAPI_DATA(PyTypeObject) PyBaseObject_Type; /* built-in 'object' */
PyAPI_DATA(PyTypeObject) PySuper_Type; /* built-in 'super' */

PyAPI_FUNC(unsigned long) PyType_GetFlags(PyTypeObject*);

PyAPI_FUNC(int) PyType_Ready(PyTypeObject *);
PyAPI_FUNC(PyObject *) PyType_GenericAlloc(PyTypeObject *, Py_ssize_t);
PyAPI_FUNC(PyObject *) PyType_GenericNew(PyTypeObject *,
                                               PyObject *, PyObject *);
PyAPI_FUNC(unsigned int) PyType_ClearCache(void);
PyAPI_FUNC(void) PyType_Modified(PyTypeObject *);

/* Generic operations on objects */
PyAPI_FUNC(PyObject *) PyObject_Repr(PyObject *);
PyAPI_FUNC(PyObject *) PyObject_Str(PyObject *);
PyAPI_FUNC(PyObject *) PyObject_ASCII(PyObject *);
PyAPI_FUNC(PyObject *) PyObject_Bytes(PyObject *);
PyAPI_FUNC(PyObject *) PyObject_RichCompare(PyObject *, PyObject *, int);
PyAPI_FUNC(int) PyObject_RichCompareBool(PyObject *, PyObject *, int);
PyAPI_FUNC(PyObject *) PyObject_GetAttrString(PyObject *, const char *);
PyAPI_FUNC(int) PyObject_SetAttrString(PyObject *, const char *, PyObject *);
PyAPI_FUNC(int) PyObject_HasAttrString(PyObject *, const char *);
PyAPI_FUNC(PyObject *) PyObject_GetAttr(PyObject *, PyObject *);
PyAPI_FUNC(int) PyObject_SetAttr(PyObject *, PyObject *, PyObject *);
PyAPI_FUNC(int) PyObject_HasAttr(PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) PyObject_SelfIter(PyObject *);
PyAPI_FUNC(PyObject *) PyObject_GenericGetAttr(PyObject *, PyObject *);
PyAPI_FUNC(int) PyObject_GenericSetAttr(PyObject *, PyObject *, PyObject *);
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03030000
PyAPI_FUNC(int) PyObject_GenericSetDict(PyObject *, PyObject *, void *);
#endif
PyAPI_FUNC(Py_hash_t) PyObject_Hash(PyObject *);
PyAPI_FUNC(Py_hash_t) PyObject_HashNotImplemented(PyObject *);
PyAPI_FUNC(int) PyObject_IsTrue(PyObject *);
PyAPI_FUNC(int) PyObject_Not(PyObject *);
PyAPI_FUNC(int) PyCallable_Check(PyObject *);
PyAPI_FUNC(void) PyObject_ClearWeakRefs(PyObject *);

/* PyObject_Dir(obj) acts like Python builtins.dir(obj), returning a
   list of strings.  PyObject_Dir(NULL) is like builtins.dir(),
   returning the names of the current locals.  In this case, if there are
   no current locals, NULL is returned, and PyErr_Occurred() is false.
*/
PyAPI_FUNC(PyObject *) PyObject_Dir(PyObject *);


/* Helpers for printing recursive container types */
PyAPI_FUNC(int) Py_ReprEnter(PyObject *);
PyAPI_FUNC(void) Py_ReprLeave(PyObject *);

/* Flag bits for printing: */
#define Py_PRINT_RAW    1       /* No string quotes etc. */

/*
Type flags (tp_flags)

These flags are used to change expected features and behavior for a
particular type.

Arbitration of the flag bit positions will need to be coordinated among
all extension writers who publicly release their extensions (this will
be fewer than you might expect!).

Most flags were removed as of Python 3.0 to make room for new flags.  (Some
flags are not for backwards compatibility but to indicate the presence of an
optional feature; these flags remain of course.)

Type definitions should use Py_TPFLAGS_DEFAULT for their tp_flags value.

Code can use PyType_HasFeature(type_ob, flag_value) to test whether the
given type object has a specified feature.
*/

#ifndef Py_LIMITED_API

/* Set to warn on creation of a __dict__ for instances of this type. */
#define Py_TPFLAGS_WARN_ON_SETATTR (1UL << 4)

/* Set if instances of the type object are treated as sequences for pattern matching */
#define Py_TPFLAGS_SEQUENCE (1 << 5)
/* Set if instances of the type object are treated as mappings for pattern matching */
#define Py_TPFLAGS_MAPPING (1 << 6)
#endif

/* Disallow creating instances of the type: set tp_new to NULL and don't create
 * the "__new__" key in the type dictionary. */
#define Py_TPFLAGS_DISALLOW_INSTANTIATION (1UL << 7)

/* Set if the type object is immutable: type attributes cannot be set nor deleted */
#define Py_TPFLAGS_IMMUTABLETYPE (1UL << 8)

/* Set if the type object is dynamically allocated */
#define Py_TPFLAGS_HEAPTYPE (1UL << 9)

/* Set if the type allows subclassing */
#define Py_TPFLAGS_BASETYPE (1UL << 10)

/* Set if the type implements the vectorcall protocol (PEP 590) */
#ifndef Py_LIMITED_API
#define Py_TPFLAGS_HAVE_VECTORCALL (1UL << 11)
// Backwards compatibility alias for API that was provisional in Python 3.8
#define _Py_TPFLAGS_HAVE_VECTORCALL Py_TPFLAGS_HAVE_VECTORCALL
#endif

/* Set if the type is 'ready' -- fully initialized */
#define Py_TPFLAGS_READY (1UL << 12)

/* Set while the type is being 'readied', to prevent recursive ready calls */
#define Py_TPFLAGS_READYING (1UL << 13)

/* Objects support garbage collection (see objimpl.h) */
#define Py_TPFLAGS_HAVE_GC (1UL << 14)

/* These two bits are preserved for Stackless Python, next after this is 17 */
#ifdef STACKLESS
#define Py_TPFLAGS_HAVE_STACKLESS_EXTENSION (3UL << 15)
#else
#define Py_TPFLAGS_HAVE_STACKLESS_EXTENSION 0
#endif

/* Objects behave like an unbound method */
#define Py_TPFLAGS_METHOD_DESCRIPTOR (1UL << 17)

/*
  No instances of this class have attributes that shadow a method.  When this
  flag is set, it allows us to potentially skip one dictionary lookup during an
  instance method call.

  Normally, method lookup for an instance method call proceeds as follows:

  1. Look for the method in the mro of the instance's type.
  2. If the result from (1) is a data descriptor, invoke it and return the
     result.
  3. Look in the instance dictionary.
  4. If (3) returned a value, return it.
  5. If (1) returned a non-data descriptor, invoke it and return the result.
  6. If (1) returned a value, return it.
  7. Raise an AttributeError

  We can bypass step 3 of this process if this flag is set on the instance's
  type and step 1 returned anything. Fortunately, this is the common case
  for method calls.

  The flag is set when a type is first created; no instances of the type exist,
  therefore no shadowing can occur. The flag maintains the invariant that it is
  set if and only if it is also set on all bases. The flag is cleared when
  shadowing may have occurred and is never reset. It must also be cleared on
  all descendent types in the hierarchy in order to maintain the invariant.

  Shadowing can occur in the following scenarios:

  1. An attribute is assigned to an instance and it shadows a method. This can
     be efficiently detected when an attribute is set on an instance.
  2. An attribute is assigned to an instance through its __dict__. We cannot
     efficiently detect when this occurs, so we assume that shadowing occurs
     whenever an instance's __dict__ is retrieved.
  3. An attribute's __dict__ is replaced. We could check for shadowing using
     each member in the __dict__, however, this should so rarely that the
     optimization is not worth the effort. We assume shadowing occurs
     whenever the __dict__ is replaced.
  4. The __class__ of an instance is re-assigned. This is effectively the
     same as (3).
  5. A method is assigned to a type. This may cause shadowing between instances
     of the type or any subtype. Knowing whether or not shadowing occurred is
     challenging to implement efficiently and would likely require keeping
     track of all the instance attribute names that were ever assigned to
     instances of a given inheritance hierarchy. However, we can be
     conservative and assume that any assignment of a method to a type causes
     shadowing.
  6. A type's __bases__ are modified. This should happen rarely and, as such,
     does not need to be handled efficiently. This is handled by (5).
*/
#define Py_TPFLAGS_NO_SHADOWING_INSTANCES (1UL << 18)

/* Object has up-to-date type attribute cache */
#define Py_TPFLAGS_VALID_VERSION_TAG  (1UL << 19)

/* Type is abstract and cannot be instantiated */
#define Py_TPFLAGS_IS_ABSTRACT (1UL << 20)

// This undocumented flag gives certain built-ins their unique pattern-matching
// behavior, which allows a single positional subpattern to match against the
// subject itself (rather than a mapped attribute on it):
#define _Py_TPFLAGS_MATCH_SELF (1UL << 22)

/* Set if type's tp_as_async slot points to PyAsyncMethodsWithExtra */
#define Py_TPFLAGS_HAVE_AM_EXTRA (1UL << 23)

/* These flags are used to determine if a type is a subclass. */
#define Py_TPFLAGS_LONG_SUBCLASS        (1UL << 24)
#define Py_TPFLAGS_LIST_SUBCLASS        (1UL << 25)
#define Py_TPFLAGS_TUPLE_SUBCLASS       (1UL << 26)
#define Py_TPFLAGS_BYTES_SUBCLASS       (1UL << 27)
#define Py_TPFLAGS_UNICODE_SUBCLASS     (1UL << 28)
#define Py_TPFLAGS_DICT_SUBCLASS        (1UL << 29)
#define Py_TPFLAGS_BASE_EXC_SUBCLASS    (1UL << 30)
#define Py_TPFLAGS_TYPE_SUBCLASS        (1UL << 31)

#define Py_TPFLAGS_DEFAULT  ( \
                 Py_TPFLAGS_HAVE_STACKLESS_EXTENSION | \
                0)

/* NOTE: Some of the following flags reuse lower bits (removed as part of the
 * Python 3.0 transition). */

/* The following flags are kept for compatibility; in previous
 * versions they indicated presence of newer tp_* fields on the
 * type struct.
 * Starting with 3.8, binary compatibility of C extensions across
 * feature releases of Python is not supported anymore (except when
 * using the stable ABI, in which all classes are created dynamically,
 * using the interpreter's memory layout.)
 * Note that older extensions using the stable ABI set these flags,
 * so the bits must not be repurposed.
 */
#define Py_TPFLAGS_HAVE_FINALIZE (1UL << 0)
#define Py_TPFLAGS_HAVE_VERSION_TAG   (1UL << 18)


/*
The macros Py_INCREF(op) and Py_DECREF(op) are used to increment or decrement
reference counts.  Py_DECREF calls the object's deallocator function when
the refcount falls to 0; for
objects that don't contain references to other objects or heap memory
this can be the standard function free().  Both macros can be used
wherever a void expression is allowed.  The argument must not be a
NULL pointer.  If it may be NULL, use Py_XINCREF/Py_XDECREF instead.
The macro _Py_NewReference(op) initialize reference counts to 1, and
in special builds (Py_REF_DEBUG, Py_TRACE_REFS) performs additional
bookkeeping appropriate to the special build.

We assume that the reference count field can never overflow; this can
be proven when the size of the field is the same as the pointer size, so
we ignore the possibility.  Provided a C int is at least 32 bits (which
is implicitly assumed in many parts of this code), that's enough for
about 2**31 references to an object.

XXX The following became out of date in Python 2.2, but I'm not sure
XXX what the full truth is now.  Certainly, heap-allocated type objects
XXX can and should be deallocated.
Type objects should never be deallocated; the type pointer in an object
is not considered to be a reference to the type object, to save
complications in the deallocation function.  (This is actually a
decision that's up to the implementer of each new type so if you want,
you can count such references to the type object.)
*/

#ifdef Py_REF_DEBUG
PyAPI_DATA(Py_ssize_t) _Py_RefTotal;
PyAPI_FUNC(void) _Py_NegativeRefcount(const char *filename, int lineno,
                                      PyObject *op);
PyAPI_FUNC(Py_ssize_t) _Py_GetRefTotal(void);
#define _Py_INC_REFTOTAL        _Py_RefTotal++
#define _Py_DEC_REFTOTAL        _Py_RefTotal--

/* Py_REF_DEBUG also controls the display of refcounts and memory block
 * allocations at the interactive prompt and at interpreter shutdown
 */
PyAPI_FUNC(void) _PyDebug_PrintTotalRefs(void);
#else
#define _Py_INC_REFTOTAL
#define _Py_DEC_REFTOTAL
#endif /* Py_REF_DEBUG */

/* Immortalizing causes the instance to not participate in reference counting.
 * The object will be kept alive until the runtime finalization.
 * This avoids an unnecessary copy-on-write for applications that share
 * a common python heap across many processes. */
#ifdef Py_IMMORTAL_INSTANCES

/*
Immortalization:

The following indicates the immortalization strategy depending on the amount
of available bits in the reference count field. All strategies are backwards
compatible but the specific reference count value or immortalization check
might change depending on the specializations for the underlying system.

Proper deallocation of immortal instances requires distinguishing between
statically allocated immortal instances vs those promoted by the runtime to be
immortal. The latter which should be the only instances that require proper
cleanup during runtime finalization.
*/

#if SIZEOF_VOID_P > 4
/*
In 64+ bit systems, an object will be marked as immortal by setting all of the
lower 32 bits of the reference count field.

i.e., in a 64 bit system the reference count will be set to:
    00000000 00000000 00000000 00000000
    11111111 11111111 11111111 11111111

Reference count increases will use saturated arithmetic, taking advantage of
having all the lower 32 bits set, which will avoid the reference count to go
beyond the refcount limit. Immortality checks for reference count decreases will
be done by checking the bit sign flag in the lower 32 bits.
*/
static const Py_ssize_t kImmortalInitialCount = UINT_MAX;
#else
/*
In 32 bit systems, an object will be marked as immortal by setting all of the
lower 30 bits of the reference count field.

i.e The reference count will be set to:
    00111111 11111111 11111111 11111111

Using the lower 30 bits makes the value backwards compatible by allowing
C-Extensions without the updated checks in Py_INCREF and Py_DECREF to safely
increase and decrease the objects reference count. The object would lose its
immortality, but the execution would still be correct.
Reference count increases and decreases will first go through an immortality
check by comparing the reference count field to the immortality reference count.
*/
static const Py_ssize_t kImmortalInitialCount = (UINT_MAX >> 2)
#endif // SIZEOF_VOID_P > 4

static inline void _Py_SetImmortal(PyObject *ob)
{
    if (ob) {
        ob->ob_refcnt = kImmortalInitialCount;
    }
}
#define Py_SET_IMMORTAL(ob) _Py_SetImmortal(_PyObject_CAST(ob))

static inline __attribute__((always_inline)) int _Py_IsImmortal(PyObject* ob) {
#if SIZEOF_VOID_P > 4
    return ((PY_INT32_T)(ob->ob_refcnt)) < 0;
#else
    return op->ob_refcnt == kImmortalInitialCount;
#endif
}

#define Py_SET_REFCNT(op, refcnt)                 \
    do {                                          \
        if(!_Py_IsImmortal(_PyObject_CAST(op)))   \
            ((PyObject *)op)->ob_refcnt = refcnt; \
    } while (0)

#else

static const Py_ssize_t kImmortalInitialCount = 1;

#define Py_SET_REFCNT(op, refcnt)  (((PyObject *)op)->ob_refcnt = refcnt)

static inline __attribute__((always_inline)) int _Py_IsImmortal(PyObject* Py_UNUSED(ob)) {
    return 0;
}

#endif // !Py_IMMORTAL_INSTANCES

PyAPI_FUNC(void) _Py_Dealloc(PyObject *);

/*
These are provided as conveniences to Python runtime embedders, so that
they can have object code that is not dependent on Python compilation flags.
*/
PyAPI_FUNC(void) Py_IncRef(PyObject *);
PyAPI_FUNC(void) Py_DecRef(PyObject *);

// Similar to Py_IncRef() and Py_DecRef() but the argument must be non-NULL.
// Private functions used by Py_INCREF() and Py_DECREF().
PyAPI_FUNC(void) _Py_IncRef(PyObject *);
PyAPI_FUNC(void) _Py_DecRef(PyObject *);

#ifdef Ci_REF_DEBUG
PyAPI_FUNC(void) Ci_RefDebug_ToggleDumpRefChanges(void);
PyAPI_FUNC(void) Ci_RefDebug_DumpInc(PyObject *op);
PyAPI_FUNC(void) Ci_RefDebug_DumpDec(PyObject *op);
#endif

static inline __attribute__((always_inline)) void _Py_INCREF(PyObject *op)
{
#ifdef Py_IMMORTAL_INSTANCES
#if SIZEOF_VOID_P > 4
    uint32_t new_refcnt;
    uint32_t cur_refcnt = (uint32_t)op->ob_refcnt;

// check immortality and return if so
#if __has_builtin(__builtin_uadd_overflow)
    // Saturated Add
    if (__builtin_uadd_overflow(cur_refcnt, 1, &new_refcnt)) {
        return;
    }
#else
    // Portable Saturated Add
    new_refcnt = cur_refcnt + 1;
    if (new_refcnt < cur_refcnt) {
        return;
    }
#endif // __has_builtin(__builtin_uadd_overflow)

#ifdef Ci_REF_DEBUG
    Ci_RefDebug_DumpInc(op);
#endif

    _Py_INC_REFTOTAL;
// copy lower 32 bits of new refcnt over to object's refcnt
#if __has_builtin(__builtin_memcpy_inline)
    __builtin_memcpy_inline(&op->ob_refcnt, &new_refcnt, sizeof(new_refcnt));
#elif __has_builtin(__builtin_memcpy)
    __builtin_memcpy(&op->ob_refcnt, &new_refcnt, sizeof(new_refcnt));
#else
    memcpy(&op->ob_refcnt, &new_refcnt, sizeof(new_refcnt));
#endif

#else // 32-bit system case
    // Explicitly check immortality against the immortal value
    if (_Py_IsImmortal(op)) {
        return;
    }
    _Py_INC_REFTOTAL;
    op->ob_refcnt++;
#endif // SIZEOF_VOID_P > 4
#else
    _Py_INC_REFTOTAL;
    op->ob_refcnt++;
#endif // Py_IMMORTAL_INSTANCES
}
#define Py_INCREF(op) _Py_INCREF(_PyObject_CAST(op))

static inline __attribute__((always_inline)) void _Py_DECREF(
#if defined(Py_REF_DEBUG) && !(defined(Py_LIMITED_API) && Py_LIMITED_API+0 >= 0x030A0000)
    const char *filename, int lineno,
#endif
    PyObject *op)
{
#ifdef Py_IMMORTAL_INSTANCES
    if (_Py_IsImmortal(_PyObject_CAST(op))) {
        return;
    }
#endif

#ifdef Ci_REF_DEBUG
    Ci_RefDebug_DumpDec(op);
#endif

#if defined(Py_REF_DEBUG) && defined(Py_LIMITED_API) && Py_LIMITED_API+0 >= 0x030A0000
    // Stable ABI for Python 3.10 built in debug mode.
    _Py_DecRef(op);
#else
    // Non-limited C API and limited C API for Python 3.9 and older access
    // directly PyObject.ob_refcnt.
    _Py_DEC_REFTOTAL;
    if (--op->ob_refcnt != 0) {
#ifdef Py_REF_DEBUG
        if (op->ob_refcnt < 0) {
            _Py_NegativeRefcount(filename, lineno, op);
        }
#endif
    }
    else {
        _Py_Dealloc(op);
    }
#endif
}
#if defined(Py_REF_DEBUG) && !(defined(Py_LIMITED_API) && Py_LIMITED_API+0 >= 0x030A0000)
#  define Py_DECREF(op) _Py_DECREF(__FILE__, __LINE__, _PyObject_CAST(op))
#else
#  define Py_DECREF(op) _Py_DECREF(_PyObject_CAST(op))
#endif


/* Safely decref `op` and set `op` to NULL, especially useful in tp_clear
 * and tp_dealloc implementations.
 *
 * Note that "the obvious" code can be deadly:
 *
 *     Py_XDECREF(op);
 *     op = NULL;
 *
 * Typically, `op` is something like self->containee, and `self` is done
 * using its `containee` member.  In the code sequence above, suppose
 * `containee` is non-NULL with a refcount of 1.  Its refcount falls to
 * 0 on the first line, which can trigger an arbitrary amount of code,
 * possibly including finalizers (like __del__ methods or weakref callbacks)
 * coded in Python, which in turn can release the GIL and allow other threads
 * to run, etc.  Such code may even invoke methods of `self` again, or cause
 * cyclic gc to trigger, but-- oops! --self->containee still points to the
 * object being torn down, and it may be in an insane state while being torn
 * down.  This has in fact been a rich historic source of miserable (rare &
 * hard-to-diagnose) segfaulting (and other) bugs.
 *
 * The safe way is:
 *
 *      Py_CLEAR(op);
 *
 * That arranges to set `op` to NULL _before_ decref'ing, so that any code
 * triggered as a side-effect of `op` getting torn down no longer believes
 * `op` points to a valid object.
 *
 * There are cases where it's safe to use the naive code, but they're brittle.
 * For example, if `op` points to a Python integer, you know that destroying
 * one of those can't cause problems -- but in part that relies on that
 * Python integers aren't currently weakly referencable.  Best practice is
 * to use Py_CLEAR() even if you can't think of a reason for why you need to.
 */
#define Py_CLEAR(op)                            \
    do {                                        \
        PyObject *_py_tmp = _PyObject_CAST(op); \
        if (_py_tmp != NULL) {                  \
            (op) = NULL;                        \
            Py_DECREF(_py_tmp);                 \
        }                                       \
    } while (0)

/* Function to use in case the object pointer can be NULL: */
static inline void _Py_XINCREF(PyObject *op)
{
    if (op != NULL) {
        Py_INCREF(op);
    }
}

#define Py_XINCREF(op) _Py_XINCREF(_PyObject_CAST(op))

static inline void _Py_XDECREF(PyObject *op)
{
    if (op != NULL) {
        Py_DECREF(op);
    }
}

#define Py_XDECREF(op) _Py_XDECREF(_PyObject_CAST(op))

// Create a new strong reference to an object:
// increment the reference count of the object and return the object.
PyAPI_FUNC(PyObject*) Py_NewRef(PyObject *obj);

// Similar to Py_NewRef(), but the object can be NULL.
PyAPI_FUNC(PyObject*) Py_XNewRef(PyObject *obj);

static inline PyObject* _Py_NewRef(PyObject *obj)
{
    Py_INCREF(obj);
    return obj;
}

static inline PyObject* _Py_XNewRef(PyObject *obj)
{
    Py_XINCREF(obj);
    return obj;
}

// Py_NewRef() and Py_XNewRef() are exported as functions for the stable ABI.
// Names overridden with macros by static inline functions for best
// performances.
#define Py_NewRef(obj) _Py_NewRef(_PyObject_CAST(obj))
#define Py_XNewRef(obj) _Py_XNewRef(_PyObject_CAST(obj))


/*
_Py_NoneStruct is an object of undefined type which can be used in contexts
where NULL (nil) is not suitable (since NULL often means 'error').

Don't forget to apply Py_INCREF() when returning this value!!!
*/
PyAPI_DATA(PyObject) _Py_NoneStruct; /* Don't use this directly */
#define Py_None (&_Py_NoneStruct)

// Test if an object is the None singleton, the same as "x is None" in Python.
PyAPI_FUNC(int) Py_IsNone(PyObject *x);
#define Py_IsNone(x) Py_Is((x), Py_None)

/* Macro for returning Py_None from a function */
#ifdef Py_IMMORTAL_INSTANCES
#define Py_RETURN_NONE return Py_None
#else
#define Py_RETURN_NONE return Py_NewRef(Py_None)
#endif

/*
Py_NotImplemented is a singleton used to signal that an operation is
not implemented for a given type combination.
*/
PyAPI_DATA(PyObject) _Py_NotImplementedStruct; /* Don't use this directly */
#define Py_NotImplemented (&_Py_NotImplementedStruct)

/* Macro for returning Py_NotImplemented from a function */
#ifdef Py_IMMORTAL_INSTANCES
#define Py_RETURN_NOTIMPLEMENTED return Py_NotImplemented
#else
#define Py_RETURN_NOTIMPLEMENTED return Py_NewRef(Py_NotImplemented)
#endif

/* Rich comparison opcodes */
#define Py_LT 0
#define Py_LE 1
#define Py_EQ 2
#define Py_NE 3
#define Py_GT 4
#define Py_GE 5

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x030A0000
/* Result of calling PyIter_Send */
typedef enum {
    PYGEN_RETURN = 0,
    PYGEN_ERROR = -1,
    PYGEN_NEXT = 1,
} PySendResult;
#endif

/*
 * Macro for implementing rich comparisons
 *
 * Needs to be a macro because any C-comparable type can be used.
 */
#define Py_RETURN_RICHCOMPARE(val1, val2, op)                               \
    do {                                                                    \
        switch (op) {                                                       \
        case Py_EQ: if ((val1) == (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_NE: if ((val1) != (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_LT: if ((val1) < (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
        case Py_GT: if ((val1) > (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
        case Py_LE: if ((val1) <= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_GE: if ((val1) >= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        default:                                                            \
            Py_UNREACHABLE();                                               \
        }                                                                   \
    } while (0)


/*
More conventions
================

Argument Checking
-----------------

Functions that take objects as arguments normally don't check for nil
arguments, but they do check the type of the argument, and return an
error if the function doesn't apply to the type.

Failure Modes
-------------

Functions may fail for a variety of reasons, including running out of
memory.  This is communicated to the caller in two ways: an error string
is set (see errors.h), and the function result differs: functions that
normally return a pointer return NULL for failure, functions returning
an integer return -1 (which could be a legal return value too!), and
other functions return 0 for success and -1 for failure.
Callers should always check for errors before using the result.  If
an error was set, the caller must either explicitly clear it, or pass
the error on to its caller.

Reference Counts
----------------

It takes a while to get used to the proper usage of reference counts.

Functions that create an object set the reference count to 1; such new
objects must be stored somewhere or destroyed again with Py_DECREF().
Some functions that 'store' objects, such as PyTuple_SetItem() and
PyList_SetItem(),
don't increment the reference count of the object, since the most
frequent use is to store a fresh object.  Functions that 'retrieve'
objects, such as PyTuple_GetItem() and PyDict_GetItemString(), also
don't increment
the reference count, since most frequently the object is only looked at
quickly.  Thus, to retrieve an object and store it again, the caller
must call Py_INCREF() explicitly.

NOTE: functions that 'consume' a reference count, like
PyList_SetItem(), consume the reference even if the object wasn't
successfully stored, to simplify error handling.

It seems attractive to make other functions that take an object as
argument consume a reference count; however, this may quickly get
confusing (even the current practice is already confusing).  Consider
it carefully, it may save lots of calls to Py_INCREF() and Py_DECREF() at
times.
*/

#ifndef Py_LIMITED_API
#  define Py_CPYTHON_OBJECT_H
#  include  "cpython/object.h"
#  undef Py_CPYTHON_OBJECT_H
#endif


static inline int
PyType_HasFeature(PyTypeObject *type, unsigned long feature)
{
    unsigned long flags;
#ifdef Py_LIMITED_API
    // PyTypeObject is opaque in the limited C API
    flags = PyType_GetFlags(type);
#else
    flags = type->tp_flags;
#endif
    return ((flags & feature) != 0);
}

#define PyType_FastSubclass(type, flag) PyType_HasFeature(type, flag)

static inline int _PyType_Check(PyObject *op) {
    return PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_TYPE_SUBCLASS);
}
#define PyType_Check(op) _PyType_Check(_PyObject_CAST(op))

static inline int _PyType_CheckExact(PyObject *op) {
    return Py_IS_TYPE(op, &PyType_Type);
}
#define PyType_CheckExact(op) _PyType_CheckExact(_PyObject_CAST(op))

CiAPI_FUNC(void) _PyType_ClearNoShadowingInstances(struct _typeobject *, PyObject *obj);
CiAPI_FUNC(void) _PyType_SetNoShadowingInstances(struct _typeobject *);

#ifdef __cplusplus
}
#endif
#endif /* !Py_OBJECT_H */
