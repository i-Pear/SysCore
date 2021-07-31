#ifndef OS_RISC_V_UNIQUEPTR_H
#define OS_RISC_V_UNIQUEPTR_H

template<typename Ptr>
class UniquePtr {
public:
    explicit UniquePtr(Ptr *ptr) : ptr_(ptr) {}

    UniquePtr(UniquePtr<Ptr> &other_ptr) {
        ptr_ = other_ptr.ptr_;
        other_ptr.ptr_ = nullptr;
    }

    virtual ~UniquePtr() {
        delete ptr_;
    }

    UniquePtr(UniquePtr<Ptr> &&other_ptr) noexcept {
        ptr_ = other_ptr.ptr_;
        other_ptr.ptr_ = nullptr;
    }

    UniquePtr &operator=(const UniquePtr<Ptr> &other_ptr) {
        if (this == &other_ptr)return *this;
        ptr_ = other_ptr.ptr_;
        other_ptr.ptr_ = nullptr;
        return *this;
    }

    Ptr operator*() {
        return *ptr_;
    }

    Ptr *operator->() {
        return ptr_;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    Ptr *ptr_;
};

#endif //OS_RISC_V_UNIQUEPTR_H
