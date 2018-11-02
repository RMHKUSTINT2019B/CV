#pragma once
#include <cmath>
#include <cstddef>
#include <utility>
#include <type_traits>
template <size_t d, class T>
union Vec;

template <class T>
union Vec<2, T> final {
    T data[2];
    struct {
        T x, y;
    };
    struct {
        T less, last;
    };
    Vec() = default;
    constexpr Vec(T _x, T _y) noexcept
            :x(_x), y(_y) { }
    constexpr Vec operator+(const Vec& r) const noexcept { return Vec(x+r.x, y+r.y); }
    constexpr Vec operator-(const Vec& r) const noexcept { return Vec(x-r.x, y-r.y); }
    template <class T2>
    constexpr Vec operator*(T2&& r) const noexcept { return Vec(x*std::forward<T2>(r), y*std::forward<T2>(r)); }
    template <class T2>
    constexpr Vec operator/(T2&& r) const noexcept { return Vec(x/std::forward<T2>(r), y/std::forward<T2>(r)); }
    Vec& operator+=(const Vec& r) noexcept {
        x += r.x;
        y += r.y;
        return *this;
    }
    Vec& operator-=(const Vec& r) noexcept {
        x -= r.x;
        y -= r.y;
        return *this;
    }
    template <class T2>
    Vec& operator*=(T2&& r) noexcept {
        x *= std::forward<T2>(r);
        y *= std::forward<T2>(r);
        return *this;
    }
    template <class T2>
    Vec& operator/=(T2&& r) noexcept {
        x /= std::forward<T2>(r);
        y /= std::forward<T2>(r);
        return *this;
    }
    constexpr T lengthSqr() const noexcept { return x*x+y*y; }
    constexpr bool operator==(const Vec& r) const noexcept { return (x==r.x) && (y==r.y); }
    constexpr bool operator<(const Vec& r) const noexcept { return lengthSqr()<r.lengthSqr(); }
    constexpr bool operator>(const Vec& r) const noexcept { return lengthSqr()>r.lengthSqr(); }
    constexpr bool operator<=(const Vec& r) const noexcept { return lengthSqr()<=r.lengthSqr(); }
    constexpr bool operator>=(const Vec& r) const noexcept { return lengthSqr()>=r.lengthSqr(); }
    constexpr T dot(const Vec& r) const noexcept { return x*r.x+y*r.y; }
    T length() noexcept { return sqrt(lengthSqr()); }
    template <class T2>
    operator Vec<2, T2>() noexcept { return Vec<2, T2>(T2(x), T2(y)); }
};

template <class T>
union Vec<3, T> final {
    T data[3];
    struct {
        T x, y, z;
    };
    struct {
        Vec<2, T> less;
        T last;
    };
    Vec() = default;
    constexpr Vec(T _x, T _y, T _z) noexcept
            :x(_x), y(_y), z(_z) { }
    constexpr Vec(const Vec<2, T>& ls, T arg) noexcept
            :less(ls), last(arg) { }
    constexpr Vec operator+(const Vec& r) const noexcept { return Vec(x+r.x, y+r.y, z+r.z); }
    constexpr Vec operator-(const Vec& r) const noexcept { return Vec(x-r.x, y-r.y, z-r.z); }
    template <class T2>
    constexpr Vec operator*(T2&& r) const noexcept {
        return Vec(x*std::forward<T2>(r), y*std::forward<T2>(r), z*std::forward<T2>(r));
    }
    //cross product
    constexpr Vec operator*(const Vec& r) const noexcept { return Vec(y*r.z-z*r.y, z*r.x-x*r.z, x*r.y-y*r.x); }
    template <class T2>
    constexpr Vec operator/(T2&& r) const noexcept {
        return Vec(x/std::forward<T2>(r), y/std::forward<T2>(r), z/std::forward<T2>(r));
    }
    Vec& operator+=(const Vec& r) noexcept {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }
    Vec& operator-=(const Vec& r) noexcept {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }
    template <class T2>
    Vec& operator*=(T2&& r) noexcept {
        x *= std::forward<T2>(r);
        y *= std::forward<T2>(r);
        z *= std::forward<T2>(r);
        return *this;
    }
    //cross product
    Vec& operator*=(const Vec& r) const noexcept {
        *this = Vec(y*r.z-z*r.y, z*r.x-x*r.z, x*r.y-y*r.x);
        return *this;
    }
    template <class T2>
    Vec& operator/=(T2&& r) noexcept {
        x /= std::forward<T2>(r);
        y /= std::forward<T2>(r);
        z /= std::forward<T2>(r);
        return *this;
    }
    constexpr T lengthSqr() const noexcept { return x*x+y*y+z*z; }
    constexpr bool operator==(const Vec& r) const noexcept { return (x==r.x) && (y==r.y) && (z==r.z); }
    constexpr bool operator<(const Vec& r) const noexcept { return lengthSqr()<r.lengthSqr(); }
    constexpr bool operator>(const Vec& r) const noexcept { return lengthSqr()>r.lengthSqr(); }
    constexpr bool operator<=(const Vec& r) const noexcept { return lengthSqr()<=r.lengthSqr(); }
    constexpr bool operator>=(const Vec& r) const noexcept { return lengthSqr()>=r.lengthSqr(); }
    constexpr T dot(const Vec& r) const noexcept { return x*r.x+y*r.y+z*r.z; }
    T length() noexcept { return sqrt(lengthSqr()); }
    template <class T2>
    operator Vec<3, T2>() noexcept { return Vec<3, T2>(T2(x), T2(y), T2(z)); }
};

template <size_t d, class T>
union Vec final {
    T data[d];
    struct {
        Vec<d-1, T> less;
        T last;
    };
    Vec() = default;
    template <class ...T2>
    constexpr Vec(const T& a1, T2&& ... args) noexcept
            :data{a1, std::forward<T2>(args)...} { }
    constexpr Vec(const Vec<d-1, T>& _less, const T& _last) noexcept
            :less(_less), last(_last) { }
    template <size_t d2, class ...T2>
    constexpr Vec(const Vec<d2, T>& ptr, const T& arg, const T& arg2, T2&& ...args) noexcept
            :Vec(Vec<d2+1, T>(ptr, arg), arg2, std::forward<T2>(args)...) { }
    constexpr Vec operator+(const Vec& r) const noexcept { return Vec(less+r.less, last+r.last); }
    constexpr Vec operator-(const Vec& r) const noexcept { return Vec(less-r.less, last-r.last); }
    template <class T2>
    constexpr Vec operator*(T2&& r) const noexcept { return Vec(less*std::forward<T2>(r), last*std::forward<T2>(r)); }
    template <class T2>
    constexpr Vec operator/(T2&& r) const noexcept { return Vec(less/std::forward<T2>(r), last/std::forward<T2>(r)); }
    Vec& operator+=(const Vec& r) noexcept {
        less += r.less;
        last += r.last;
        return *this;
    }
    Vec& operator-=(const Vec& r) noexcept {
        less -= r.less;
        last -= r.last;
        return *this;
    }
    template <class T2>
    Vec& operator*=(T2&& r) noexcept {
        less *= std::forward<T2>(r);
        last *= std::forward<T2>(r);
        return *this;
    }
    template <class T2>
    Vec& operator/=(T2&& r) noexcept {
        less /= std::forward<T2>(r);
        last /= std::forward<T2>(r);
        return *this;
    }
    constexpr T lengthSqr() const noexcept { return less.lengthSqr()+last*last; }
    constexpr bool operator==(const Vec& r) const noexcept { return (less==r.less) && (last==r.last); }
    constexpr bool operator<(const Vec& r) const noexcept { return lengthSqr()<r.lengthSqr(); }
    constexpr bool operator>(const Vec& r) const noexcept { return lengthSqr()>r.lengthSqr(); }
    constexpr bool operator<=(const Vec& r) const noexcept { return lengthSqr()<=r.lengthSqr(); }
    constexpr bool operator>=(const Vec& r) const noexcept { return lengthSqr()>=r.lengthSqr(); }
    constexpr T dot(const Vec& r) const noexcept { return less.dot(r.less)+last*r.last; }
    T length() noexcept { return sqrt(lengthSqr()); }
    template <class T2>
    operator Vec<d, T2>() noexcept { return Vec<d, T2>(less, T2(last)); }
};

using Vec2i = Vec<2, int>;
using Vec2f = Vec<2, float>;
using Vec2d = Vec<2, double>;
template <class T>
using Vec2 = Vec<2, T>;
using Vec3i = Vec<3, int>;
using Vec3f = Vec<3, float>;
using Vec3d = Vec<3, double>;
template <class T>
using Vec3 = Vec<3, T>;
