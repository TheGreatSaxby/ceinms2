/*This is a simple test from the official boost documentation to assess whether boost libraries have
 * been installed correctly
 */

#include <boost/math/differentiation/autodiff.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/constants/constants.hpp>
#include <iostream>
#include <vector>
#include <tuple>
#include "ceinms2/Lloyd2003Muscle.h"

using namespace boost::math::differentiation;

template<typename T>
T fourth_power(T const &x) {
    T x4 = x * x;// retval in operator*() uses x4's memory via NRVO.
    x4 *= x4;// No copies of x4 are made within operator*=() even when squaring.
    return x4;// x4 uses y's memory in main() via NRVO.
}

bool test1() {

    constexpr unsigned Order = 5;// Highest order derivative to be calculated.
    auto const x = make_fvar<double, Order>(2.0);// Find derivatives at x=2.
    auto const y = fourth_power(x);
    const std::vector<double> expected{ 16., 32., 48., 48., 24., 0. };
    bool failed = false;
    for (unsigned i = 0; i <= Order; ++i) {
        std::cout << "y.derivative(" << i << ") = " << y.derivative(i) << std::endl;
        failed |= (y.derivative(i) != expected.at(i));
    }
    return failed;
}



template<typename W, typename X, typename Y, typename Z>
auto f(const W &w, const X &x, const Y &y, const Z &z) {
    using namespace std;
    return exp(w * sin(x * log(y) / z) + sqrt(w * z / (x * y))) + w * w / tan(z);
}

bool test2() {
    using float50 = boost::multiprecision::cpp_bin_float_50;

    constexpr unsigned Nw = 3;// Max order of derivative to calculate for w
    constexpr unsigned Nx = 2;// Max order of derivative to calculate for x
    constexpr unsigned Ny = 4;// Max order of derivative to calculate for y
    constexpr unsigned Nz = 3;// Max order of derivative to calculate for z
    // Declare 4 independent variables together into a std::tuple.
    auto const variables = make_ftuple<float50, Nw, Nx, Ny, Nz>(11, 12, 13, 14);
    auto const &w = std::get<0>(variables);// Up to Nw derivatives at w=11
    auto const &x = std::get<1>(variables);// Up to Nx derivatives at x=12
    auto const &y = std::get<2>(variables);// Up to Ny derivatives at y=13
    auto const &z = std::get<3>(variables);// Up to Nz derivatives at z=14
    auto const v = f(w, x, y, z);
    // Calculated from Mathematica symbolic differentiation.
    float50 const answer("1976.319600747797717779881875290418720908121189218755");
    std::cout << std::setprecision(std::numeric_limits<float50>::digits10)
              << "mathematica   : " << answer << '\n'
              << "autodiff      : " << v.derivative(Nw, Nx, Ny, Nz) << '\n'
              << std::setprecision(3)
              << "relative error: " << (v.derivative(Nw, Nx, Ny, Nz) / answer - 1) << '\n';
    return 0;
}


template<typename X, typename Y>
auto test_fun(const X& x, const Y& y) {
    auto val = x *x *x *x + 2 * y *y;
    return val;
}

bool test3() {
    constexpr unsigned Order = 1;// Highest order derivative to be calculated.
    auto const variables = make_ftuple<double, 2, 2>(2, 2);
    auto const &x = std::get<0>(variables);
    auto const &y = std::get<1>(variables);
    auto const z = test_fun(x, y);
    std::cout << z.at(0, 0) << std::endl;
    std::cout << z.at(1, 0) << std::endl;
    std::cout << z.at(0, 1) << std::endl;

    bool failed = false;
    failed |= (z.at(0, 0) != 24);
    failed |= (z.at(1, 0) != 32);
    failed |= (z.at(0, 1) != 8);

    return failed;

}

bool test4() {
    const size_t N = 1000;
    const double dt = 0.1;
    const double w = boost::math::double_constants::two_pi * 3.;
    std::vector<double> x, y;
    for (size_t i{ 0 }; i < N; ++i) {
        const double t{ i * dt };
        x.push_back(t);
        y.push_back(std::cos(w * t));
    }

    // Get derivative through object-provided method
    ceinms::CubicSpline spline(x, y);

    bool failed = false;
    for (size_t i{ 0 }; i < N; ++i) {
        const double t{ i * dt * 0.9 };
        auto yp = spline.getFirstDerivative(t);
        // get derivative using autodiff
        auto const xx = make_fvar<double, 1>(t);
        auto yy = spline.get(xx);
        failed |= (yy.derivative(1) != yp);
    }
    return failed;
}

int main() {
    bool failed = false;
    failed |= test1();
    failed |= test2();
    failed |= test3();
    failed |= test4();
    return failed;
}


