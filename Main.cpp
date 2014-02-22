

#include "NewThread.hpp"
#include <pthread.h>
#include <tuple>
#include <set>
#include <deque>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include <unistd.h>
#include <cstdint>

using namespace std;
using namespace cs540;



ostream &
operator<<(ostream &os, const type_info &ti) {
    int ec;
    const char *demangled_name = abi::__cxa_demangle(ti.name(), 0, 0, &ec);
    if (ec != 0) {
        cerr << "Demangling failure: ";
        switch (ec) {
            case -1:
                cerr << "Memory allocation failure." << endl;
                break;
            case -2:
                cerr << "Invalid mangled name: " << ti.name() << endl;
                break;
            case -3:
                cerr << "Inalid args" << endl;
                break;
            default:
                cerr << "Unknown demanglig error" << endl;
                break;
        }
        assert(false); abort();
    }
    os << demangled_name;
    free((void *) demangled_name);
    return os;
}

class A {
    public:
        int get(int) {
            cout << "Called" << endl;
            return 1;
        }
        double foo(const char *, int) {
            cout << "Foo called" << endl;
            return 1.0;
        }
        double foo(const char *, double) {
            cout << "Foo2 called" << endl;
            return 1.0;
        }
};

class MyClass {
    public:
        MyClass();
        ~MyClass();
        void method();
        const set<pthread_t> &verify() const {
            return m_verify;
        }
    private:
        set<pthread_t> m_verify;
        pthread_mutex_t m_mutex;
};

MyClass::MyClass() {
    int ec;
    ec = pthread_mutex_init(&m_mutex, 0); assert(ec == 0);
}

MyClass::~MyClass() {
    int ec;
    ec = pthread_mutex_destroy(&m_mutex); assert(ec == 0);
}

void
MyClass::method() {
    int ec;
    pthread_t tid = pthread_self();
    ec = pthread_mutex_lock(&m_mutex); assert(ec == 0);
    auto res = m_verify.insert(tid);
    ec = pthread_mutex_unlock(&m_mutex); assert(ec == 0);
    // Must have successfully inserted, since there
    // should be no duplicates.
    assert(res.second);
}

void *
spawn(void *vp) {

    unsigned int seed = (uintptr_t(vp));

    deque<pthread_t> threads;
    MyClass obj;

    int n = 100*(double(rand_r(&seed))/RAND_MAX);
    for (int i = 0; i < n; i++) {
        pthread_t tid = NewThread(&obj, &MyClass::method);
        threads.push_back(tid);
    }
    for (int i = 0; i < n; i++) {
        pthread_t tid = threads.at(i);
        int ec = pthread_join(tid, 0); assert(ec == 0);
    }
    auto &verify(obj.verify());
    for (int i = 0; i < n; i++) {
        assert(verify.find(threads.at(i)) != verify.end());
    }

    return nullptr;
}

int main() {

    A a;
    int ec;

    pthread_t tid = NewThread(&a, static_cast<double (A::*)(const char *, double)>(&A::foo), "hello", 1.0);
    //pthread_t tid = NewThread(&a, &A::foo, "hello", 1.0);
    ec = pthread_join(tid, 0); assert(ec == 0);
    sleep(1);

    spawn(nullptr);
}
