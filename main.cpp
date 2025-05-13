
#include "softposit_cpp.h"
#include <Eigen/Dense>
#include <chrono>

namespace Eigen 
{
    template<>
    struct NumTraits<posit16> {
        using Self = posit16;
        using Real = posit16;
        using NonInteger = posit16;
        using Nested = posit16;
        using Literal = float;
    
        enum {
            IsComplex = 0,
            IsInteger = 0,
            IsSigned = 1,
            RequireInitialization = 1,
            ReadCost = 1,
            AddCost = 2,
            MulCost = 2
        };
    
        static inline Real epsilon() { return p16(0.00001f); }
        static inline Real dummy_precision() { return p16(0.00001f); }
        static inline int digits10() { return 3; }  // arbitrary safe num
    };

    template<>
    struct NumTraits<posit32> {
        using Self = posit32;
        using Real = posit32;
        using NonInteger = posit32;
        using Nested = posit32;
        using Literal = float;
    
        enum {
            IsComplex = 0,
            IsInteger = 0,
            IsSigned = 1,
            RequireInitialization = 1,
            ReadCost = 1,
            AddCost = 2,
            MulCost = 2
        };
    
        static inline Real epsilon() { return p32(0.00001f); }
        static inline Real dummy_precision() { return p32(0.00001f); }
        static inline int digits10() { return 3; }  // arbitrary safe num
    };
}

template<typename A, typename B>
void benchmark(int r, int c, int repetitions, A&& numa, B&& numb)
{
    assert(repetitions > 0);
    using namespace std::chrono;
    using namespace Eigen;

    Matrix<posit32, Dynamic, Dynamic> pa(r, c);
    Matrix<posit32, Dynamic, Dynamic> pb(r, c);

    Matrix<float, Dynamic, Dynamic> fa(r, c);
    Matrix<float, Dynamic, Dynamic> fb(r, c);

    Matrix<double, Dynamic, Dynamic> da(r, c);
    Matrix<double, Dynamic, Dynamic> db(r, c);

    pa.fill(p32(std::forward<A>(numa)));
    pb.fill(p32(std::forward<B>(numb)));
    fa.fill(float(std::forward<A>(numa)));
    fb.fill(float(std::forward<B>(numb)));
    da.fill(std::forward<A>(numa));
    db.fill(std::forward<B>(numb));

    duration<double, std::micro> pelapsed; 
    duration<double, std::micro> felapsed;
    double posit_mean_error{};
    double float_mean_error{};
    for(int i {}; i < repetitions; ++i)
    {
        auto pstart = high_resolution_clock::now();
        auto pmul = pa * pb;
        volatile auto padd = pa + pb;
        volatile auto pmin = pa - pb;
        auto pend = high_resolution_clock::now();
        pelapsed += pend - pstart;

        auto fstart = high_resolution_clock::now();
        auto fmul = fa * fb;
        volatile auto fadd = fa + fb;
        volatile auto fmin = fa - fb;
        auto fend = high_resolution_clock::now();
        felapsed += fend - fstart;
        
        // calculate error
        Matrix<double, Dynamic, Dynamic> ref = da * db;
        Matrix<double, Dynamic, Dynamic> p_to_d(r, c);

        for(int row{}; row < pmul.rows(); ++row) {
            for(int col{}; col < pmul.cols(); ++col) {
                p_to_d(row, col) = pmul(row, col).toDouble();
            }
        }

        if (!fmul.allFinite()) {
            std::cerr << "Float result has NaN or Inf at size " << r << "x" << c << "\n";
            return;
        }
        if (!pa.allFinite() || !pb.allFinite()) {
            std::cerr << "Posit input matrices are invalid at size " << r << "x" << c << "\n";
            return;
        }

        auto posit_abs_error = (ref - p_to_d).cwiseAbs();
        auto float_abs_error = (ref - fmul.cast<double>()).cwiseAbs();
        posit_mean_error += posit_abs_error.mean();
        float_mean_error += float_abs_error.mean();
    }
    pelapsed /= repetitions;
    felapsed /= repetitions;
    posit_mean_error /= repetitions;
    float_mean_error /= repetitions;

    std::cout << "\t--------Matrix Size: " << 
    r << "x" << c << "--------\n";
    std::cout << "\t Posit Time taken: " << pelapsed.count() << "\n";
    std::cout << "\t Float Time taken: " << felapsed.count() << "\n";
    std::cout << "\t Posit Mean Absolute Error: " << posit_mean_error << "\n";
    std::cout << "\t Float Mean Absolute Error: " << float_mean_error << "\n";
}

int main()
{
    #ifdef EIGEN_VECTORIZE_SSE
        std::cout << "SSE enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_AVX
        std::cout << "AVX enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_AVX512
        std::cout << "AVX-512 enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_NEON
        std::cout << "NEON enabled (ARM)\n";
    #endif
    #ifndef EIGEN_VECTORIZE
        std::cout << "No SIMD vectorization\n";
    #endif

    for(int i{ 10 }; i <= 50; i += 10)
    {
        benchmark(i, i, 5, 1.0, 2.0);
        benchmark(i, i, 5, 1.00001, 0.99999);
        benchmark(i, i, 5, 1e-5, 2e-5);
        benchmark(i, i, 5, 1e4, 1e4);
    }
      
    return 0;
}