/* Custom block allocator for use with several Microsoft VC++ 5.0 STL
 * containers.
 *
 * block_allocator is a custom STL allocator for use with STL as
 * implemented in Microsoft VC++ 5.0. Rather than doing allocations
 * on a per-node basis, block_allocator allocates memory in fixed sized
 * chunks and delivers portions of these chunks as requested. Typical speed
 * improvements of 40% have been obtained with respect to the default
 * allocator. The size of the chunks, set by the user, should not be too
 * little (reduced speed improvement) nor too large (memory wasted).
 * Experiment and see what sizes fit best to your application.
 *
 * block_allocator can substitute for the default allocator in the following
 * containers:
 *
 *   · list,
 *   · set,
 *   · multiset,
 *   · map,
 *   · multimap,
 *
 * and WON'T work with other containers such as vector or queue. Note
 * however that vector and queue already perform allocation in chunks.
 * For each of the containers supported, there's an associated block allocated
 * container derived from it thru use of block_allocator. You have to define an
 * activating macro for each container to be defined prior to the inclusion
 * of blockallocator.h (more info near the end of this header):
 *
 *   · list     -> block_allocated_list     (macro DEFINE_BLOCK_ALLOCATED_LIST),
 *   · set      -> block_allocated_set      (macro DEFINE_BLOCK_ALLOCATED_SET),
 *   · multiset -> block_allocated_multiset (macro DEFINE_BLOCK_ALLOCATED_MULTISET),
 *   · map      -> block_allocated_map      (macro DEFINE_BLOCK_ALLOCATED_MAP),
 *   · multimap -> block_allocated_multimap (macro DEFINE_BLOCK_ALLOCATED_MULTIMAP),
 *
 * To use block allocation based STL in your application, define the 
 * corresponding activating macro, include blockallocator.h and then change
 * your declarations as follows:
 *
 *   list<type>         -> block_allocated_list<type,chunk_size>
 *   set<key>           -> block_allocated_set<key,chunk_size>
 *   multiset<key>      -> block_allocated_multiset<key,chunk_size>
 *   map<key,type>      -> block_allocated_map<key,type,chunk_size>
 *   multimap<key,type> -> block_allocated_multimap<key,type,chunk_size>
 *
 * where chunk_size is the size of the chunks. You can enter too the
 * other optional template parameters (see MSVC++ STL docs for more info).
 * Normal containers and block allocated containers can coexist without
 * problems.
 *
 * This code has been tested for MSVC++ 5.0/6.0.
 *
 * This code won't work with other STL implementations different from 
 * Microsoft's. This is due to the fact that Microsoft STL relies on
 * a non-standard extension to the allocator interface to cope with
 * the lack of template member function support of Microsoft C++ compiler.
 *
 * Multithreading issues: Each block allocated container instance uses its own
 * block allocator, so no multithreading problems should arise as long as your
 * program conveniently protects their containers for concurrent access (or if no
 * two threads access the same container instance). This is the same scenario posed
 * by regular STL classes (remember operations on containers are not guarded by
 * CRITICAL_SECTIONs or anything similar), so the moral of it all is: If your program
 * was multithread safe without block_allocator, it'll continue to be with it.
 *
 * Modifications to v1.0:
 *   · Version 1.0 didn't compile in MSVC++ 6.0 due to a compiler bug. A
 *     description of this (not yet fixed) bug can be found in
 *
 *       BUG: C2233 on User-Defined Type Array Member of Template Class,
 *       Microsoft Knowledge Base Article Q216977,
 *       http://support.microsoft.com/support/kb/articles/Q216/9/77.ASP
 *
 *     Fortunately enough, there's a purely syntactic nifty workaround for
 *     the problem that does not incur in any performance or memory penalty
 *     and preserves the layout of the classes involved. The workaround
 *     was found by Doug Harrison (dHarrison@worldnet.att.net) and is
 *     superior to the crap suggested by Microsoft as a fixup. You can
 *     find Doug's contribution in USENET article
 *
 *       "Re: Status on Visual C++ 6.0 Template Bug [C2233]?",
 *       microsoft.public.vc.language, 04/27/99
 *
 * Modifications to v1.0:
 *   · Included definitions for operator== and operator!=. The lack of these
 *     caused linking errors when invoking list::swap() and similar methods.
 *
 * LEGAL:
 *
 * (C) 2000-2001 Joaquín Mª López Muñoz (joaquin@tid.es). All rights reserved.
 *
 * Permission is granted to use, distribute and modify this code provided that:
 *   · this copyright notice remain unchanged,
 *   · you submit all changes to the copyright holder and properly mark the
 *     changes so they can be told from the original code,
 *   · credits are given to the copyright holder in the documentation of any
 *     software using this code with the following line:
 *       "Portions copyright 2000-2001 Joaquín Mª López Muñoz (joaquin@tid.es)"
 *
 * The author welcomes any suggestions on the code or reportings of actual
 * use of the code. Please send your comments to joaquin@tid.es.
 *
 * The author makes NO WARRANTY or representation, either express or implied,
 * with respect to this code, its quality, accuracy, merchantability, or
 * fitness for a particular purpose.  This software is provided "AS IS", and
 * you, its user, assume the entire risk as to its quality and accuracy.
 *
 * Last modified: September 30th, 2001
 */

#ifndef BLOCKALLOCATOR_H
#define BLOCKALLOCATOR_H

#define VERSION_BLOCKALLOCATOR 0x00010002

#include <stddef.h>
#include <assert.h>

/* forward declaration */

template <size_t node_size,size_t chunk_size> struct block_allocator_chunk;

/* A cell has information on the chunk it belongs in and a pointer
 * for keeping a linked list of free cells. To save space, next and
 * node have been merged into a union (next is only used when the
 * cell is not used by the caller).
 */

template <size_t node_size,size_t chunk_size> struct block_allocator_cell
{
  block_allocator_chunk<node_size,chunk_size>  *pchunk;
  union{
    block_allocator_cell<node_size,chunk_size> *next;
    char                                        node[node_size];
  };
};

/* A chunk has pointers to other chunks to keep a double linked
 * list of non-full chunks, as well as a pointer to its first
 * available cell.
 */

/* Define block_allocator_chunk_header separately so that we
 * can place dummy elements at the beginning and end of the linked list
 * whitout the overhead of the data.
 */

template <size_t node_size,size_t chunk_size> struct block_allocator_chunk_header
{
  block_allocator_chunk<node_size,chunk_size> *previous;
  block_allocator_chunk<node_size,chunk_size> *next;

  block_allocator_chunk_header(block_allocator_chunk<node_size,chunk_size> *previous=0,
                               block_allocator_chunk<node_size,chunk_size> *next=0):
    previous(previous),
    next(next)
  {
  }
};

template <size_t node_size,size_t chunk_size> struct block_allocator_chunk:
  public block_allocator_chunk_header<node_size,chunk_size>
{
  size_t                                       num_used_cells;
  block_allocator_cell<node_size,chunk_size>  *pfirst_available_cell;

  /* workaround for MSVC++ 6.0 bug. See "Modifications to v1.0" paragraph */

  enum{_chunk_size=chunk_size};
  block_allocator_cell<node_size,_chunk_size>  cells[chunk_size];

  /* The ctor puts the object in a state in which the first
   * cell is already "allocated". This saves some instructions
   * in the code.
   */

  block_allocator_chunk(block_allocator_chunk<node_size,chunk_size> *previous,
                        block_allocator_chunk<node_size,chunk_size> *next):
    block_allocator_chunk_header<node_size,chunk_size>(previous,next),
    num_used_cells(1),
    pfirst_available_cell(&cells[1])
  {
    /* link it */

    previous->next=next->previous=this;

    /* initialize cells */

    cells[0].pchunk=this;

    cells[chunk_size-1].pchunk=this;
    cells[chunk_size-1].next=0;

    block_allocator_cell<node_size,chunk_size> *pcell=&cells[1];
    block_allocator_cell<node_size,chunk_size> *pnext_cell=&cells[2];

    for(size_t n=chunk_size-2;n--;){
      pcell->pchunk=this;
      pcell++->next=pnext_cell++;
    }
  }
};

/* The template arguments of block_allocator are:
 *   · T, the type of the elements contained.
 *   · N, a class whose sizeof() determines the number of bytes
 *     the allocator will be requested in every call to _Charalloc(),
 *   · chunk_size, the number of nodes per chunk.
 */

template <class T,class N,size_t chunk_size> class block_allocator
{
  public:
    /* Standard definitions, borrowed from the default VC++ allocator */

    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef T         *pointer;
    typedef const T   *const_pointer;
    typedef T&         reference;
    typedef const T&   const_reference;
    typedef T          value_type;

    pointer address(reference x)const{return &x;}
    const_pointer address(const_reference x)const{return &x;}

    block_allocator()
    {
      head.next=reinterpret_cast<chunk *>(&tail);
      tail.previous=reinterpret_cast<chunk *>(&head);

      /* The code assumes chunk_size>=2. If that's not the case, it
       * will crash for sure.
       */

      assert(chunk_size>=2);
    }

    block_allocator(const block_allocator&)
    {
      /* Don't care about the argument and proceed as if in the default ctor.
       * This is consistent with the behavior of operator== (see).
       */

      head.next=reinterpret_cast<chunk *>(&tail);
      tail.previous=reinterpret_cast<chunk *>(&head);
      assert(chunk_size>=2);
    }

    /* same considerations as in  block_allocator(const block_allocator&) */

    block_allocator<T,N,chunk_size>& operator=(const block_allocator<T,N,chunk_size>&)
    {
      return *this;
    }

    /* containers supported never call this one */

    pointer allocate(size_type n,const void *hint)
    {
      assert(false);
      return 0;
    }

    char *_Charalloc(size_type n)
    {
      assert(n==sizeof(N));

      cell *pcell;

      if(head.next==&tail){                         /* no chunks available */
        new chunk(reinterpret_cast<chunk *>(&head), /* get a new one */
                  reinterpret_cast<chunk *>(&tail));
        pcell=&head.next->cells[0];                 /* use its first cell */
      }
      else{
        pcell=head.next->pfirst_available_cell; /* get the cell */
        ++head.next->num_used_cells;            /* and log it */
        if((head.next->pfirst_available_cell=pcell->next)==0){
          /* no more cells available in this chunk, delink it */

          head.next=head.next->next;
          head.next->previous=reinterpret_cast<chunk *>(&head);
        }
      }

      /* return the node portion of the cell */

      return reinterpret_cast<char *>(&pcell->node);
    }

    void deallocate(void *p,size_type n)
    {
      assert(p!=0&&n==1);

      cell *pcell=reinterpret_cast<cell *>(reinterpret_cast<char *>(p)-offsetof(cell,node));
      chunk *pchunk=pcell->pchunk;

      if(--pchunk->num_used_cells==0){ 
        /* all cells in this chunk disposed of, delete and delink */

        pchunk->previous->next=pchunk->next;
        pchunk->next->previous=pchunk->previous;
        delete pchunk;
      }
      else if((pcell->next=pchunk->pfirst_available_cell)==0){
        pchunk->pfirst_available_cell=pcell;

        /* all cells in this chunk were in use and this is the first
         * that gets deallocated, link the chunk at the beginning of
         * the list.
         * NB: We don't need set pchunk->previous: it already pointed
         * to head when the chunk got delinked.
         */

        pchunk->next=head.next;
        head.next=head.next->previous=pchunk;
      }
      else{ /* link the cell to the list of available cells for this chunk */
        pchunk->pfirst_available_cell=pcell;
      }
    }

    /* standard stuff */

    void construct(pointer p,const T& val)
    {
      new ((void *)p) T(val);
    }

    void destroy(pointer p)
    {
      p->T::~T();
    }

    size_type max_size()const
    {
      size_type n=(size_type)(-1)/sizeof(N);
      return (0<n?n:1);
    }

    /* Every two instances are treated as different */

    bool operator ==(const block_allocator<T,N,chunk_size> &r) const
    {
        return this==&r;
    }

    bool operator !=(const block_allocator<T,N,chunk_size> &r) const
    {
        return this!=&r;
    }

    /* This two member functions never get called, but VC complains if no
     * declaration for them is provided. See documentation for this
     * VC++ confirmed bug in the Microsoft Knowledge Base, article Q166721.
     * I think this has been fixed in VC++ 6.0.
     */

    bool operator <(const block_allocator<T,N,chunk_size> &) const;
    bool operator >(const block_allocator<T,N,chunk_size> &) const;

  private:
    /* some typing saving typedefs */

    typedef block_allocator_cell<sizeof(N),chunk_size>         cell;
    typedef block_allocator_chunk_header<sizeof(N),chunk_size> chunk_header;
    typedef block_allocator_chunk<sizeof(N),chunk_size>        chunk;

    /* dummy elements at the beginning and end of the list of available
     * chunks. We could have done whithout these, but they allow for a
     * slightly more efficient code.
     */

    chunk_header head;
    chunk_header tail;
};

/* We keep the following definitions out of the multiple include preventing
 * #endif, so that different include's can be used to define different
 * block allocated containers, as in this example:
 *
 *  #define DEFINE_BLOCK_ALLOCATED_LIST
 *  #include "blockallocator.h"
 *
 *  #define DEFINE_BLOCK_ALLOCATED_MULTIMAP
 *  #include "blockallocator.h"
 *
 */

#elif VERSION_BLOCKALLOCATOR!=0x00010002
#error You have included two BLOCKALLOCATOR.H with different version numbers
#endif

/* Definitions for each container supported. These need the corresponding
 * STL headers to be included, so they come with associated macros
 * to turn them on only in the case you really want them.
 * Each definition comes with a helper struct named MSVC_STL_XXX_node.
 * This struct mimics the layout of the objects for which the container
 * will request memory to our allocator, so they are used to calculate the
 * size of the nodes required.
 */

#ifdef DEFINE_BLOCK_ALLOCATED_LIST
#ifndef BLOCK_ALLOCATED_LIST_DEFINED
#define BLOCK_ALLOCATED_LIST_DEFINED

#pragma warning(disable:4786)
#include<list>

#pragma pack(push,8)

template <class T> struct MSVC_STL_list_node
{
  void *p1,*p2;
  T     t;
};

#pragma pack(pop)

template <class T,size_t chunk_size>
class block_allocated_list: 
  public std::list<T,block_allocator<T,MSVC_STL_list_node<T>,chunk_size> >
{
  public:
    explicit block_allocated_list(
      const allocator_type& al=block_allocator<T,MSVC_STL_list_node<T>,chunk_size>()
    ):
      std::list<T,block_allocator<T,MSVC_STL_list_node<T>,chunk_size> >(al)
    {
    }

    explicit block_allocated_list(
      size_type n,const T& v=T(),
      const allocator_type& al=block_allocator<T,MSVC_STL_list_node<T>,chunk_size>()
    ):
      std::list<T,block_allocator<T,MSVC_STL_list_node<T>,chunk_size> >(n,v,al)
    {
    }

    block_allocated_list(
      const_iterator first,const_iterator last,
      const allocator_type& al=block_allocator<T,MSVC_STL_list_node<T>,chunk_size>()
    ):
      std::list<T,block_allocator<T,MSVC_STL_list_node<T>,chunk_size> >(first,last,al)
    {
    }
};

#endif
#endif /* DEFINE_BLOCK_ALLOCATED_LIST */

#ifdef DEFINE_BLOCK_ALLOCATED_SET
#ifndef BLOCK_ALLOCATED_SET_DEFINED
#define BLOCK_ALLOCATED_SET_DEFINED

#pragma warning(disable:4786)
#include<set>

#pragma pack(push,8)

template <class T> struct MSVC_STL_set_node
{
  void *p1,*p2,*p3;
  T     t;
  int   i;
};

#pragma pack(pop)

template <class Key,size_t chunk_size,class Pred=std::less<Key> >
class block_allocated_set: 
  public std::set<Key,Pred,
                  block_allocator<Key,MSVC_STL_set_node<Key>,chunk_size> >
{
  public:
    explicit block_allocated_set(
      const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<Key,MSVC_STL_set_node<Key>,chunk_size>()
    ):
      std::set<Key,Pred,block_allocator<Key,MSVC_STL_set_node<Key>,chunk_size> >(comp,al)
    {
    }

    block_allocated_set(
      const value_type *first,const value_type *last,const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<Key,MSVC_STL_set_node<Key>,chunk_size>()
    ):
      std::set<Key,Pred,block_allocator<Key,MSVC_STL_set_node<Key>,chunk_size> >
        (first,last,comp,al)
    {
    }
};

#endif
#endif /* DEFINE_BLOCK_ALLOCATED_SET */

#ifdef DEFINE_BLOCK_ALLOCATED_MULTISET
#ifndef BLOCK_ALLOCATED_MULTISET_DEFINED
#define BLOCK_ALLOCATED_MULTISET_DEFINED

#pragma warning(disable:4786)
#include<set>

#pragma pack(push,8)

template <class T> struct MSVC_STL_multiset_node
{
  void *p1,*p2,*p3;
  T     t;
  int   i;
};

#pragma pack(pop)

template <class Key,size_t chunk_size,class Pred=std::less<Key> >
class block_allocated_multiset: 
  public std::multiset<Key,Pred,
                       block_allocator<Key,MSVC_STL_multiset_node<Key>,chunk_size> >
{
  public:
    explicit block_allocated_multiset(
      const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<Key,MSVC_STL_multiset_node<Key>,chunk_size>()
    ):
      std::multiset<Key,Pred,block_allocator<Key,MSVC_STL_multiset_node<Key>,chunk_size> >
        (comp,al)
    {
    }

    block_allocated_multiset(
      const value_type *first,const value_type *last,const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<Key,MSVC_STL_multiset_node<Key>,chunk_size>()
    ):
      std::multiset<Key,Pred,block_allocator<Key,MSVC_STL_multiset_node<Key>,chunk_size> >
        (first,last,comp,al)
    {
    }
};

#endif
#endif /* DEFINE_BLOCK_ALLOCATED_MULTISET */

#ifdef DEFINE_BLOCK_ALLOCATED_MAP
#ifndef BLOCK_ALLOCATED_MAP_DEFINED
#define BLOCK_ALLOCATED_MAP_DEFINED

#pragma warning(disable:4786)
#include<map>

#pragma pack(push,8)

template <class Key,class T> struct MSVC_STL_map_node
{
  void            *p1,*p2,*p3;
  std::pair<Key,T> t;
  int              i;
};

#pragma pack(pop)

template<class Key,class T,size_t chunk_size,class Pred=std::less<Key> >
class block_allocated_map:
  public std::map<Key,T,Pred,
                  block_allocator<T,MSVC_STL_map_node<Key,T>,chunk_size> >
{
  public:
    explicit block_allocated_map(
      const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<T,MSVC_STL_map_node<Key,T>,chunk_size>()
    ):
      std::map<Key,T,Pred,block_allocator<T,MSVC_STL_map_node<Key,T>,chunk_size> >(comp,al)
    {
    }

    block_allocated_map(
      const value_type *first,const value_type *last,const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<T,MSVC_STL_map_node<Key,T>,chunk_size>()
    ):
      std::map<Key,T,Pred,block_allocator<T,MSVC_STL_map_node<Key,T>,chunk_size> >
        (first,last,comp,al)
    {
    }
};

#endif
#endif /* DEFINE_BLOCK_ALLOCATED_MAP */

#ifdef DEFINE_BLOCK_ALLOCATED_MULTIMAP
#ifndef BLOCK_ALLOCATED_MULTIMAP_DEFINED
#define BLOCK_ALLOCATED_MULTIMAP_DEFINED

#pragma warning(disable:4786)
#include<map>

#pragma pack(push,8)

template <class Key,class T> struct MSVC_STL_multimap_node
{
  void            *p1,*p2,*p3;
  std::pair<Key,T> t;
  int              i;
};

#pragma pack(pop)

template<class Key,class T,size_t chunk_size,class Pred=std::less<Key> >
class block_allocated_multimap:
  public std::multimap<Key,T,Pred,
                       block_allocator<T,MSVC_STL_multimap_node<Key,T>,chunk_size> >
{
  public:
    explicit block_allocated_multimap(
      const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<T,MSVC_STL_multimap_node<Key,T>,chunk_size>()
    ):
      std::multimap<Key,T,Pred,block_allocator<T,MSVC_STL_multimap_node<Key,T>,chunk_size> >
        (comp,al)
    {
    }

    block_allocated_multimap(
      const value_type *first,const value_type *last,const Pred& comp=Pred(),
      const allocator_type& al=block_allocator<T,MSVC_STL_multimap_node<Key,T>,chunk_size>()
    ):
      std::multimap<Key,T,Pred,block_allocator<T,MSVC_STL_multimap_node<Key,T>,chunk_size> >
        (first,last,comp,al)
    {
    }
};

#endif
#endif /* DEFINE_BLOCK_ALLOCATED_MULTIMAP */