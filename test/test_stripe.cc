#include <iostream>

#include "stripe.h"

#include <iostream>
#include <vector>

#define BOOST_TEST_MODULE test stripe
#include <boost/test/included/unit_test.hpp>

using namespace praster;

BOOST_AUTO_TEST_CASE(StripeTileLoad) {
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

  {
    stripe stripe =
        stripe::build(0, 0, 7, ds_width, ds_height, -1, 3, 4, reader_callback);

    auto &tiles = stripe.get_tiles();

    BOOST_REQUIRE_EQUAL(tiles.size(), 8);

    {
      auto &tile = tiles[0];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[1];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[2];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 6);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[3];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 9);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[4];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[5];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[6];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 6);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[7];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 9);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 4);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }
  }

  {
    stripe stripe =
        stripe::build(1, 7, 2, ds_width, ds_height, -1, 3, 2, reader_callback);

    auto &tiles = stripe.get_tiles();

    BOOST_REQUIRE_EQUAL(tiles.size(), 4);

    {
      auto &tile = tiles[0];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 7);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 2);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[1];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 7);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 2);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[2];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 6);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 7);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 3);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 2);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }

    {
      auto &tile = tiles[3];

      BOOST_REQUIRE_EQUAL(tile.get_x(), 9);
      BOOST_REQUIRE_EQUAL(tile.get_y(), 7);
      BOOST_REQUIRE_EQUAL(tile.get_width(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_height(), 2);
      BOOST_REQUIRE_EQUAL(tile.get_left_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_right_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_top_margin(), 1);
      BOOST_REQUIRE_EQUAL(tile.get_bottom_margin(), 0);
      BOOST_REQUIRE_EQUAL(tile.get_nodata(), -1);
    }
  }
}