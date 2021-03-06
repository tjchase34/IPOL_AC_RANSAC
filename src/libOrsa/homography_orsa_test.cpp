// Copyright (c) 2007-2011 libmv authors.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <iostream>
#include "homography_model.hpp"
#include "orsa.hpp"
#include "CppUnitLite/TestHarness.h"

#define EXPECT_MATRIX_NEAR(a, b, tolerance) \
do { \
  bool dims_match = (a.nrow() == b.nrow()) && (a.ncol() == b.ncol()); \
  CHECK_EQUAL(a.nrow(),b.nrow()); \
  CHECK_EQUAL(a.ncol(),b.ncol()); \
  if (dims_match) { \
    for (int r = 0; r < a.nrow(); ++r) { \
      for (int c = 0; c < a.ncol(); ++c) { \
        DOUBLES_EQUAL(a(r, c), b(r, c), tolerance); \
      } \
    } \
  } \
} while(false);

TEST(RobustHomographyEstimation, ORSA) {
  //------------------------//
  //-- Unit test Objective--//
  //------------------------//
  // Given two points set linked by a known homography :
  // -=> Set an outlier in the point dataset.
  // -=> Make a robust estimation powered with ORSA
  // -=> Check the estimated matrix and inliers indexes.
  //--
  //--

  typedef libNumerics::matrix<double> Mat;

  // Define a synthetic dataset
  // - 15 points that are inlier,
  // - 1 point that is outlier.
  const int n = 16;
  Mat x1(2,n);
  Mat x2(2,n);
  Mat matGT(3,3); // Ground truth matrix.
  {
    // Defines input points
    double points[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,   5,
                        0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,   5};
    x1.read(points);

    x2 = x1;
    for (int i = 0; i < n; ++i) {
      x2(0, i) += i + 2; // Multiple horizontal disparities.
    }

    // Last point set as an outlier
    x2(0, n - 1) = 10;
    x2(1, n - 1) = 10;

    // Setup Ground truth matrix.
    double matGTd[] = { 4, 1, 2,  0, 1, 0,  0, 0, 1};
    matGT.read(matGTd);
  }

  // Robust homography solving:
  {
    std::vector<Match> m(n);
    for(int i=0; i<n; i++)
        m[i] = Match(x1(0,i), x1(1,i), x2(0,i), x2(1,i));
    orsa::HomographyModel kernel(m);
    double alpha0Left  = M_PI/(6*(double)6);
    double alpha0Right = M_PI/(6*(double)6);
    orsa::Orsa orsa(&kernel, alpha0Left, alpha0Right);

    std::vector<int> vec_inliers;
    Mat homographyMat(3,3);

    orsa.run(vec_inliers, 100, NULL, &homographyMat, true);

    // Assert we found 15 inliers
    CHECK(vec_inliers.size() == 15);

    //-- Re-estimate Homography on all inliers.
    kernel.ComputeModel(vec_inliers, &homographyMat);

    // Compare matrix to Ground truth.
    EXPECT_MATRIX_NEAR(matGT, homographyMat, 1e-8);
  }
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */

