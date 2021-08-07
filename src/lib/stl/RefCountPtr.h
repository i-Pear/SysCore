#ifndef OS_RISC_V_REFCOUNTPTR_H
#define OS_RISC_V_REFCOUNTPTR_H

template<typename Ptr>
class RefCountPtr {
public:

    // never use an uninitialized RefCountPtr !
    explicit RefCountPtr():atomic_count_(nullptr){
    }

    explicit RefCountPtr(Ptr* ptr):atomic_count_(new AtomicCount(ptr, 1)) {}

    virtual ~RefCountPtr() {
        decrease();
    }

    RefCountPtr(const RefCountPtr<Ptr>& other_ptr) : atomic_count_(other_ptr.atomic_count_) {
        increase();
    }

    RefCountPtr& operator=(const RefCountPtr<Ptr>& other_ptr) {
        if (this == &other_ptr)return *this;
        decrease();
        atomic_count_ = other_ptr.atomic_count_;
        increase();
        return *this;
    }

    Ptr* getPtr() {
        return atomic_count_->ptr();
    }

    Ptr* operator->() {
        return atomic_count_->ptr();
    }

    Ptr& operator*() {
        return *atomic_count_->ptr();
    }

    const Ptr& operator*() const{
        return *atomic_count_->ptr();
    }

    explicit operator bool() const {
        return atomic_count_->ptr() != nullptr;
    }

private:
    void increase() {
        if(atomic_count_){
            atomic_count_->inc();
        }
    }

    void decrease() {
        if(atomic_count_){
            atomic_count_->dec();
        }
        if (atomic_count_ && atomic_count_->count() == 0) {
            release();
        }
    }

    void release() {
        delete atomic_count_;
    }

    class AtomicCount {
    public:
        AtomicCount(Ptr* ptr, int count) : ptr_(ptr), count_(count) {};

        virtual ~AtomicCount() {
            delete ptr_;
            ptr_ = nullptr;
        }

        void inc() {
            ++count_;
        }

        void dec() {
            --count_;
        }

        Ptr* ptr() {
            return ptr_;
        }

        int count() {
            return count_;
        }

    private:
        int count_;
        Ptr* ptr_;
    };

    AtomicCount* atomic_count_;
};

#endif //OS_RISC_V_REFCOUNTPTR_H
