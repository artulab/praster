#include <iostream>

#include "tile.h"

#include <iostream>
#include <vector>

#define BOOST_TEST_MODULE test tile
#include <boost/test/included/unit_test.hpp>

using namespace praster;

BOOST_AUTO_TEST_CASE(TileMove) {
  const double nodata = -1.0;
  const int x = 1;
  const int y = 2;
  const int width = 10;
  const int height = 11;
  const int left = 5;
  const int right = 6;
  const int top = 7;
  const int bottom = 8;

  tile t1 = tile(x, y, width, height, left, right, top, bottom, nodata);
  tile t2 = std::move(t1);

  BOOST_REQUIRE_EQUAL(x, t2.get_x());
  BOOST_REQUIRE_EQUAL(y, t2.get_y());
  BOOST_REQUIRE_EQUAL(width, t2.get_width());
  BOOST_REQUIRE_EQUAL(height, t2.get_height());
  BOOST_REQUIRE_EQUAL(left, t2.get_left_margin());
  BOOST_REQUIRE_EQUAL(right, t2.get_right_margin());
  BOOST_REQUIRE_EQUAL(top, t2.get_top_margin());
  BOOST_REQUIRE_EQUAL(bottom, t2.get_bottom_margin());
  BOOST_REQUIRE_EQUAL(nodata, t2.get_nodata());
  BOOST_REQUIRE_EQUAL(true, t2.has_buffer());

  BOOST_REQUIRE_EQUAL(0, t1.get_x());
  BOOST_REQUIRE_EQUAL(0, t1.get_y());
  BOOST_REQUIRE_EQUAL(0, t1.get_width());
  BOOST_REQUIRE_EQUAL(0, t1.get_height());
  BOOST_REQUIRE_EQUAL(0, t1.get_left_margin());
  BOOST_REQUIRE_EQUAL(0, t1.get_right_margin());
  BOOST_REQUIRE_EQUAL(0, t1.get_top_margin());
  BOOST_REQUIRE_EQUAL(0, t1.get_bottom_margin());
  BOOST_REQUIRE_EQUAL(0.0, t1.get_nodata());
  BOOST_REQUIRE_EQUAL(false, t1.has_buffer());
}

BOOST_AUTO_TEST_CASE(TileForeachGet) {
  const int ds_width = 10;
  const int ds_height = 9;
  gsl_matrix *matrix = gsl_matrix_alloc(ds_height, ds_width);

  int c = 0;
  for (int i = 0; i < ds_height; ++i) {
    for (int j = 0; j < ds_width; ++j) {
      gsl_matrix_set(matrix, i, j, c++);
    }
  }

  auto reader_callback = [matrix](int x, int y, int width, int height,
                                  double *buffer) {
    gsl_matrix_view view = gsl_matrix_submatrix(matrix, y, x, height, width);
    gsl_matrix *view_matrix = &view.matrix;
    int index = 0;
    for (int i = 0; i < height; ++i) {
      for (int j = 0; j < width; ++j) {
        buffer[index++] = gsl_matrix_get(view_matrix, i, j);
      }
    }
  };

  // test loading all dataset
  {
    const int width = ds_width;
    const int height = ds_height;

    std::vector<double> expected(width * height);
    reader_callback(0, 0, width, height, &expected[0]);

    tile t = tile(0, 0, width, height, 0, 0, 0, 0, -1.0, reader_callback);

    std::vector<double> actual;

    t.foreach (
        [&t, &actual](auto &index) { actual.push_back(t.get_value(index)); });

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test loading the inner region of the dataset
  {
    const int width = 3;
    const int height = 2;
    const int x = 3;
    const int y = 4;

    std::vector<double> expected(width * height);
    reader_callback(x, y, width, height, &expected[0]);

    tile t = tile(x, y, width, height, 1, 1, 1, 1, -1.0, reader_callback);

    std::vector<double> actual;

    t.foreach (
        [&t, &actual](auto &index) { actual.push_back(t.get_value(index)); });

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test neighbours - in the middle
  {
    const int width = 3;
    const int height = 2;
    const int x = 3;
    const int y = 4;

    tile t = tile(x, y, width, height, 1, 1, 1, 1, -1.0, reader_callback);

    std::array<double, 8> actual = t.get_neighbours(0, 0);
    std::array<double, 8> expected = {32, 33, 34, 44, 54, 53, 52, 42};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test neighbours - in the left-top corner
  {
    const int width = 3;
    const int height = 2;
    const int x = 0;
    const int y = 0;

    tile t = tile(x, y, width, height, 0, 1, 0, 1, -1.0, reader_callback);

    std::array<double, 8> actual = t.get_neighbours(0, 0);
    std::array<double, 8> expected = {-1.0, -1.0, -1.0, 1, 11, 10, -1.0, -1.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test neighbours - in the right-top corner
  {
    const int width = 3;
    const int height = 2;
    const int x = ds_width - width;
    const int y = 0;

    tile t = tile(x, y, width, height, 1, 0, 0, 1, -1.0, reader_callback);

    std::array<double, 8> actual = t.get_neighbours(0, width - 1);

    std::array<double, 8> expected = {-1.0, -1.0, -1.0, -1.0, -1.0, 19, 18, 8};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test neighbours - in the right-bottom corner
  {
    const int width = 3;
    const int height = 2;
    const int x = ds_width - width;
    const int y = ds_height - height;

    tile t = tile(x, y, width, height, 1, 0, 1, 0, -1.0, reader_callback);

    std::array<double, 8> actual = t.get_neighbours(height - 1, width - 1);

    std::array<double, 8> expected = {78, 79, -1.0, -1.0, -1.0, -1.0, -1.0, 88};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }

  // test neighbours - in the left-bottom corner
  {
    const int width = 3;
    const int height = 2;
    const int x = 0;
    const int y = ds_height - height;

    tile t = tile(x, y, width, height, 0, 1, 1, 0, -1.0, reader_callback);

    std::array<double, 8> actual = t.get_neighbours(height - 1, 0);

    std::array<double, 8> expected = {-1.0, 70, 71, 81, -1.0, -1.0, -1.0, -1.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
  }
}

auto create_test_matrix(int ds_height, int ds_width) {
  gsl_matrix *matrix = gsl_matrix_alloc(ds_height, ds_width);

  for (int i = 0; i < ds_height; ++i) {
    for (int j = 0; j < ds_width; ++j) {
      int c_i = i / 3;
      int c_j = j / 3;
      int c = c_i * 3 + c_j;
      gsl_matrix_set(matrix, i, j, c);
    }
  }

  return matrix;
}

auto construct_reader_callback(gsl_matrix *matrix) {
  return [matrix](int x, int y, int width, int height, double *buffer) {
    gsl_matrix_view view = gsl_matrix_submatrix(matrix, y, x, height, width);
    gsl_matrix *view_matrix = &view.matrix;
    int index = 0;
    for (int i = 0; i < height; ++i) {
      for (int j = 0; j < width; ++j) {
        buffer[index++] = gsl_matrix_get(view_matrix, i, j);
      }
    }
  };
}

BOOST_AUTO_TEST_CASE(TileUpdateBorder) {
  // tile in the center
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);

    tile t_topleft =
        tile(0, 0, width, height, 0, 1, 0, 1, -1.0, reader_callback);
    t_topleft.transform([](double value) { return 100; });

    tile t_top = tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 101; });

    tile t_topright =
        tile(6, 0, width, height, 1, 0, 0, 1, -1.0, reader_callback);
    t_topright.transform([](double value) { return 102; });

    tile t_left = tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);
    t_left.transform([](double value) { return 103; });

    tile t_right = tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);
    t_right.transform([](double value) { return 105; });

    tile t_bottomleft =
        tile(0, 6, width, height, 0, 1, 1, 0, -1.0, reader_callback);
    t_bottomleft.transform([](double value) { return 106; });

    tile t_bottom =
        tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 107; });

    tile t_bottomright =
        tile(6, 6, width, height, 1, 0, 1, 0, -1.0, reader_callback);
    t_bottomright.transform([](double value) { return 108; });

    t_center.update_borders(&t_topleft, &t_top, &t_topright, &t_left, &t_right,
                            &t_bottomleft, &t_bottom, &t_bottomright);

    auto actual = t_center.to_vector(true);
    auto expected = std::vector<double>{100, 101, 101, 101, 102, 103, 4,   4, 4,
                                        105, 103, 4,   4,   4,   105, 103, 4, 4,
                                        4,   105, 106, 107, 107, 107, 108};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the top-left
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(0, 0, width, height, 0, 1, 0, 1, -1.0, reader_callback);

    tile t_right = tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);
    t_right.transform([](double value) { return 101; });

    tile t_bottom =
        tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 103; });

    tile t_bottomright =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_bottomright.transform([](double value) { return 104; });

    t_center.update_borders(nullptr, nullptr, nullptr, nullptr, &t_right,
                            nullptr, &t_bottom, &t_bottomright);

    auto actual = t_center.to_vector(true);
    auto expected = std::vector<double>{0, 0, 0, 101, 0,   0,   0,   101,
                                        0, 0, 0, 101, 103, 103, 103, 104};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the top-right
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(6, 0, width, height, 1, 0, 0, 1, -1.0, reader_callback);

    tile t_left = tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);
    t_left.transform([](double value) { return 101; });

    tile t_bottomleft =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_bottomleft.transform([](double value) { return 104; });

    tile t_bottom =
        tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 105; });

    t_center.update_borders(nullptr, nullptr, nullptr, &t_left, nullptr,
                            &t_bottomleft, &t_bottom, nullptr);

    auto actual = t_center.to_vector(true);
    auto expected = std::vector<double>{101, 2, 2, 2, 101, 2,   2,   2,
                                        101, 2, 2, 2, 104, 105, 105, 105};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the bottom-left
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(0, 6, width, height, 0, 1, 1, 0, -1.0, reader_callback);

    tile t_top = tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 103; });

    tile t_topright =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_topright.transform([](double value) { return 104; });

    tile t_right = tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);
    t_right.transform([](double value) { return 107; });

    t_center.update_borders(nullptr, &t_top, &t_topright, nullptr, &t_right,
                            nullptr, nullptr, nullptr);

    auto actual = t_center.to_vector(true);
    auto expected = std::vector<double>{103, 103, 103, 104, 6, 6, 6, 107,
                                        6,   6,   6,   107, 6, 6, 6, 107};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the bottom-right
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(6, 6, width, height, 1, 0, 1, 0, -1.0, reader_callback);

    tile t_topleft =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_topleft.transform([](double value) { return 104; });

    tile t_top = tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 105; });

    tile t_left = tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);
    t_left.transform([](double value) { return 107; });

    t_center.update_borders(&t_topleft, &t_top, nullptr, &t_left, nullptr,
                            nullptr, nullptr, nullptr);

    auto actual = t_center.to_vector(true);
    auto expected = std::vector<double>{104, 105, 105, 105, 107, 8, 8, 8,
                                        107, 8,   8,   8,   107, 8, 8, 8};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the middle of the top
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);

    tile t_left = tile(0, 0, width, height, 0, 1, 0, 1, -1.0, reader_callback);
    t_left.transform([](double value) { return 100; });

    tile t_bottomleft =
        tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);
    t_bottomleft.transform([](double value) { return 103; });

    tile t_bottom =
        tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 104; });

    tile t_bottomright =
        tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);
    t_bottomright.transform([](double value) { return 105; });

    tile t_right = tile(6, 0, width, height, 1, 0, 0, 1, -1.0, reader_callback);
    t_right.transform([](double value) { return 102; });

    t_center.update_borders(nullptr, nullptr, nullptr, &t_left, &t_right,
                            &t_bottomleft, &t_bottom, &t_bottomright);

    auto actual = t_center.to_vector(true);
    auto expected =
        std::vector<double>{100, 1, 1, 1, 102, 100, 1,   1,   1,   102,
                            100, 1, 1, 1, 102, 103, 104, 104, 104, 105};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the middle of the left
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);

    tile t_top = tile(0, 0, width, height, 0, 1, 0, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 100; });

    tile t_topright =
        tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);
    t_topright.transform([](double value) { return 101; });

    tile t_right = tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_right.transform([](double value) { return 104; });

    tile t_bottomright =
        tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);
    t_bottomright.transform([](double value) { return 107; });

    tile t_bottom =
        tile(0, 6, width, height, 0, 1, 1, 0, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 106; });

    t_center.update_borders(nullptr, &t_top, &t_topright, nullptr, &t_right,
                            nullptr, &t_bottom, &t_bottomright);

    auto actual = t_center.to_vector(true);
    auto expected =
        std::vector<double>{100, 100, 100, 101, 3, 3,   3,   104, 3,   3,
                            3,   104, 3,   3,   3, 104, 106, 106, 106, 107};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the middle of the bottom
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);

    tile t_topleft =
        tile(0, 3, width, height, 0, 1, 1, 1, -1.0, reader_callback);
    t_topleft.transform([](double value) { return 103; });

    tile t_top = tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 104; });

    tile t_topright =
        tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);
    t_topright.transform([](double value) { return 105; });

    tile t_left = tile(0, 6, width, height, 0, 1, 1, 0, -1.0, reader_callback);
    t_left.transform([](double value) { return 106; });

    tile t_right = tile(6, 6, width, height, 1, 0, 1, 0, -1.0, reader_callback);
    t_right.transform([](double value) { return 108; });

    t_center.update_borders(&t_topleft, &t_top, &t_topright, &t_left, &t_right,
                            nullptr, nullptr, nullptr);

    auto actual = t_center.to_vector(true);
    auto expected =
        std::vector<double>{103, 104, 104, 104, 105, 106, 7, 7, 7, 108,
                            106, 7,   7,   7,   108, 106, 7, 7, 7, 108};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }

  // tile in the middle of the right
  {
    const int ds_width = 9;
    const int ds_height = 9;

    gsl_matrix *matrix = create_test_matrix(ds_height, ds_width);
    auto reader_callback = construct_reader_callback(matrix);

    const int width = 3;
    const int height = 3;

    tile t_center =
        tile(6, 3, width, height, 1, 0, 1, 1, -1.0, reader_callback);

    tile t_topleft =
        tile(3, 0, width, height, 1, 1, 0, 1, -1.0, reader_callback);
    t_topleft.transform([](double value) { return 101; });

    tile t_top = tile(6, 0, width, height, 1, 0, 0, 1, -1.0, reader_callback);
    t_top.transform([](double value) { return 102; });

    tile t_left = tile(3, 3, width, height, 1, 1, 1, 1, -1.0, reader_callback);
    t_left.transform([](double value) { return 104; });

    tile t_bottomleft =
        tile(3, 6, width, height, 1, 1, 1, 0, -1.0, reader_callback);
    t_bottomleft.transform([](double value) { return 107; });

    tile t_bottom =
        tile(6, 6, width, height, 1, 0, 1, 0, -1.0, reader_callback);
    t_bottom.transform([](double value) { return 108; });

    t_center.update_borders(&t_topleft, &t_top, nullptr, &t_left, nullptr,
                            &t_bottomleft, &t_bottom, nullptr);

    auto actual = t_center.to_vector(true);
    auto expected =
        std::vector<double>{101, 102, 102, 102, 104, 5, 5,   5,   104, 5,
                            5,   5,   104, 5,   5,   5, 107, 108, 108, 108};

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());

    gsl_matrix_free(matrix);
  }
}