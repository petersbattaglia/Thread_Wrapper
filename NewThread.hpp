#include <tuple>
#include <pthread.h>

namespace cs540 
{

  void* holder(void *);

  class Base 
  {
    public:
      virtual void invoker() {}
      virtual ~Base() {}
  };

  // The below definitions of succ and seq were borrowed from Stack Overflow.
  template<int ...>
  struct seq {};

  template<int N, int ...S>
  struct succ : succ<N-1, N-1, S...> {};

  template<int ...S>
  struct succ<0, S...> 
  {
    typedef seq<S...> Q;
  };
  // End S.O. borrow.


  template 
  <typename T, 
  typename R, 
  typename ...Params>
  class Package : public Base 
  {
    public:

      pthread_t thread;
      
      Package(T *in, R (T::*in_vals)(Params...), Params... a) : parameters(a...)
      {
        container = in;
        vals = in_vals;
      }

      template<int... S>
      void call_thread(seq<S...>) 
      {
        (container->*vals)(std::get<S>(parameters)...);
      }

      void invoker() 
      {
        call_thread(typename succ<sizeof...(Params)>::Q()); // From S.O.
      }

      pthread_t create() 
      {
        return pthread_create(&thread, NULL, holder, this);
      }

      ~Package() { }

      private:
        std::tuple<Params...> parameters;
        R (T::*vals)(Params...);
        T *container;
  };

  template <typename T, typename S, typename ...Params>
  pthread_t NewThread(T *container, S (T::*vals)(Params...), Params... parameters) 
  {
    Package<T, S, Params...> *pkg = new Package<T, S, Params...>(container, vals, parameters...);
    (*pkg).create();
    return (*pkg).thread;
  }

  void *holder(void *p) 
  {
    Base &b = *((Base *)(p));
    b.invoker();
  }
}