#pragma once

#ifdef CLINGOLPX_USE_IMATH

// TODO: fix!!!

#include <imrat.h>

class Rational {
public:
    Rational() { // NOLINT
        handle_error_(mp_rat_init(&num_));
    }
    Rational(mp_small val)
    : Rational() {
        handle_error_(mp_rat_set_value(&num_, val, 1));
    }
    Rational(char const *val, mp_size radix)
    : Rational() {
        handle_error_(mp_rat_read_string(&num_, radix, val));
    }
    Rational(std::string const &val, mp_size radix)
    : Rational(val.c_str(), radix) {
    }
    Rational(Rational const &a)
    : Rational() {
        handle_error_(mp_rat_copy(&a.num_, &num_));
    }
    Rational(Rational &&a) noexcept
    : Rational() {
        swap(a);
    }
    Rational &operator=(Rational const &a) {
        handle_error_(mp_rat_copy(&a.num_, &num_));
        return *this;
    }
    Rational &operator=(Rational &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Rational() {
        mp_rat_clear(&num_);
    }

    void swap(Rational &x) {
        mp_int_swap(mp_rat_numer_ref(&num_), mp_rat_numer_ref(&x.num_));
        mp_int_swap(mp_rat_denom_ref(&num_), mp_rat_denom_ref(&x.num_));
    }

    void canonicalize() {
        handle_error_(mp_rat_reduce(&num_));
    }

    friend Rational operator*(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator*=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator/(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_div(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator/=(Rational &a, Rational const &b) {
        handle_error_(mp_rat_div(&a.num_, &b.num_, &a.num_));
        return a;
    }

    friend Rational operator+(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator+=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator-(Rational const &a) {
        Rational b;
        handle_error_(mp_rat_neg(&a.num_, &b.num_));
        return b;
    }
    friend Rational operator-(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator-=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend bool operator<(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) < 0;
    }
    friend bool operator<=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) <= 0;
    }
    friend bool operator>(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) > 0;
    }
    friend bool operator>=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) >= 0;
    }
    friend bool operator==(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) == 0;
    }
    friend bool operator!=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) != 0;
    }
    friend bool operator<(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) < 0;
    }
    friend bool operator<=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) <= 0;
    }
    friend bool operator>(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) > 0;
    }
    friend bool operator>=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) >= 0;
    }
    friend bool operator==(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) == 0;
    }
    friend bool operator!=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) != 0;
    }

    friend int compare(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_);
    }

    friend std::ostream &operator<<(std::ostream &out, Rational const &a) {
        constexpr int radix = 10;
        if (mp_int_compare_value(mp_rat_denom_ref(&a.num_), 1) == 0) {
            auto len = mp_int_string_len(mp_rat_numer_ref(&a.num_), radix);
            auto buf = std::make_unique<char[]>(len); // NOLINT
            handle_error_(mp_int_to_string(mp_rat_numer_ref(&a.num_), radix, buf.get(), len));
            out << buf.get();
        }
        else {
            auto len = mp_rat_string_len(&a.num_, radix);
            auto buf = std::make_unique<char[]>(len); // NOLINT
            handle_error_(mp_rat_to_string(&a.num_, radix, buf.get(), len));
            out << buf.get();
        }
        return out;
    }

private:
    static void handle_error_(mp_result res) {
        if (res != MP_OK) {
            if (res == MP_MEMORY) {
                throw std::bad_alloc();
            }
            if (res == MP_RANGE || res == MP_TRUNC) {
                throw std::range_error(mp_error_string(res));
            }
            if (res == MP_UNDEF) {
                throw std::domain_error(mp_error_string(res));
            }
            if (res == MP_BADARG) {
                throw std::invalid_argument(mp_error_string(res));
            }
            throw std::logic_error(mp_error_string(res));
        }
    }

    mutable mpq_t num_;
};

#endif
