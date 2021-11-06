#include "tile.h"

namespace praster {

tile::tile(int x, int y, int width, int height, int left_margin,
           int right_margin, int top_margin, int bottom_margin, double nodata)
    : m_x(x), m_y(y), m_width(width), m_height(height),
      m_left_margin(left_margin), m_right_margin(right_margin),
      m_top_margin(top_margin), m_bottom_margin(bottom_margin),
      m_nodata(nodata) {

  int width_with_margin = m_width + m_left_margin + m_right_margin;
  int height_with_margin = m_height + m_top_margin + m_bottom_margin;

  m_matrix = gsl_matrix_alloc(height_with_margin, width_with_margin);
}

tile::tile(
    int x, int y, int width, int height, int left_margin, int right_margin,
    int top_margin, int bottom_margin, double nodata,
    std::function<void(int x, int y, int width, int height, double *buffer)>
        read_callback)
    : tile(x, y, width, height, left_margin, right_margin, top_margin,
           bottom_margin, nodata) {

  int width_with_margin = m_width + m_left_margin + m_right_margin;
  int height_with_margin = m_height + m_top_margin + m_bottom_margin;

  read_callback(m_x - m_left_margin, m_y - m_top_margin, width_with_margin,
                height_with_margin, m_matrix->data);
}

tile::~tile() {
  if (m_matrix != nullptr) {
    gsl_matrix_free(m_matrix);
    m_matrix = nullptr;
  }
}

tile::tile(tile &&other) noexcept { *this = std::move(other); }

tile &tile::operator=(tile &&other) noexcept {
  if (this != &other) {
    m_matrix = other.m_matrix;

    m_left_margin = other.m_left_margin;
    m_right_margin = other.m_right_margin;

    m_top_margin = other.m_top_margin;
    m_bottom_margin = other.m_bottom_margin;

    m_x = other.m_x;
    m_y = other.m_y;

    m_width = other.m_width;
    m_height = other.m_height;

    m_nodata = other.m_nodata;

    other.m_matrix = nullptr;

    other.m_left_margin = 0;
    other.m_right_margin = 0;

    other.m_top_margin = 0;
    other.m_bottom_margin = 0;

    other.m_x = 0;
    other.m_y = 0;

    other.m_width = 0;
    other.m_height = 0;

    other.m_nodata = 0.0;
  }
  return *this;
}

std::vector<double> tile::to_vector(bool with_border) {
  if (with_border) {
    std::vector<double> result;
    for (int y = 0; y < m_height + m_top_margin + m_bottom_margin; ++y) {
      for (int x = 0; x < m_width + m_left_margin + m_right_margin; ++x) {
        result.push_back(gsl_matrix_get(m_matrix, y, x));
      }
    }
    return result;
  } else {
    std::vector<double> result;
    foreach ([&result](double val) { result.push_back(val); })
      ;
    return result;
  }
}

void tile::foreach (
    std::function<void(const std::pair<int, int> &index)> callback) {
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      callback(std::make_pair(y, x));
    }
  }
}

void tile::foreach (std::function<void(const int y, const int x)> callback) {
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      callback(y, x);
    }
  }
}

void tile::foreach (std::function<void(double val)> callback) {
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      callback(gsl_matrix_get(m_matrix, y + m_top_margin, x + m_left_margin));
    }
  }
}

void tile::transform(std::function<double(double val)> callback) {
  int y_index, x_index;
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      y_index = y + m_top_margin;
      x_index = x + m_left_margin;

      gsl_matrix_set(m_matrix, y_index, x_index,
                     callback(gsl_matrix_get(m_matrix, y_index, x_index)));
    }
  }
}

void tile::transform(
    std::function<double(const int y, const int x, double val)> callback) {
  int y_index, x_index;
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      y_index = y + m_top_margin;
      x_index = x + m_left_margin;

      gsl_matrix_set(
          m_matrix, y_index, x_index,
          callback(y, x, gsl_matrix_get(m_matrix, y_index, x_index)));
    }
  }
}

void tile::transform(
    std::function<double(const std::pair<int, int> &index, double val)>
        callback) {
  int y_index, x_index;
  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      y_index = y + m_top_margin;
      x_index = x + m_left_margin;

      gsl_matrix_set(m_matrix, y_index, x_index,
                     callback(std::make_pair(y, x),
                              gsl_matrix_get(m_matrix, y_index, x_index)));
    }
  }
}

void tile::update_borders(tile *top_left_tile, tile *top_tile,
                          tile *top_right_tile, tile *left_tile,
                          tile *right_tile, tile *bottom_left_tile,
                          tile *bottom_tile, tile *bottom_right_tile) {
  // update top border
  if (top_tile) {
    assert(top_tile->m_bottom_margin == 1);
    assert(m_top_margin == 1);

    gsl_vector_view neighbour = gsl_matrix_row(
        top_tile->m_matrix, top_tile->m_height + top_tile->m_top_margin - 1);
    gsl_vector_view border = gsl_matrix_row(m_matrix, 0);

    gsl_vector_memcpy(&border.vector, &neighbour.vector);
  }

  // update bottom border
  if (bottom_tile) {
    assert(bottom_tile->m_top_margin == 1);
    assert(m_bottom_margin == 1);

    gsl_vector_view neighbour = gsl_matrix_row(bottom_tile->m_matrix, 1);
    gsl_vector_view border = gsl_matrix_row(m_matrix, m_height + m_top_margin);

    gsl_vector_memcpy(&border.vector, &neighbour.vector);
  }

  // update left border
  if (left_tile) {
    assert(left_tile->m_right_margin == 1);
    assert(m_left_margin == 1);

    gsl_vector_view neighbour = gsl_matrix_column(
        left_tile->m_matrix, left_tile->m_width + left_tile->m_left_margin - 1);
    gsl_vector_view border = gsl_matrix_column(m_matrix, 0);

    gsl_vector_memcpy(&border.vector, &neighbour.vector);
  }

  // update right border
  if (right_tile) {
    assert(right_tile->m_left_margin == 1);
    assert(m_right_margin == 1);

    gsl_vector_view neighbour = gsl_matrix_column(right_tile->m_matrix, 1);
    gsl_vector_view border =
        gsl_matrix_column(m_matrix, m_width + m_left_margin);

    gsl_vector_memcpy(&border.vector, &neighbour.vector);
  }

  // update top-left border
  if (top_left_tile) {
    assert(top_left_tile->m_bottom_margin == 1 &&
           top_left_tile->m_right_margin == 1);
    assert(m_top_margin == 1 && m_left_margin == 1);

    gsl_matrix_set(
        m_matrix, 0, 0,
        gsl_matrix_get(
            top_left_tile->m_matrix,
            top_left_tile->m_height + top_left_tile->m_top_margin - 1,
            top_left_tile->m_width + top_left_tile->m_left_margin - 1));
  }

  // update top-right border
  if (top_right_tile) {
    assert(top_right_tile->m_bottom_margin == 1 &&
           top_right_tile->m_left_margin == 1);
    assert(m_top_margin == 1 && m_right_margin == 1);

    gsl_matrix_set(m_matrix, 0, m_width + m_left_margin,
                   gsl_matrix_get(top_right_tile->m_matrix,
                                  top_right_tile->m_height +
                                      top_right_tile->m_top_margin - 1,
                                  1));
  }

  // update bottom-left border
  if (bottom_left_tile) {
    assert(bottom_left_tile->m_top_margin == 1 &&
           bottom_left_tile->m_right_margin == 1);
    assert(m_bottom_margin == 1 && m_left_margin == 1);

    gsl_matrix_set(m_matrix, m_height + m_top_margin, 0,
                   gsl_matrix_get(bottom_left_tile->m_matrix, 1,
                                  bottom_left_tile->m_width +
                                      bottom_left_tile->m_left_margin - 1));
  }

  // update bottom-right border
  if (bottom_right_tile) {
    assert(bottom_right_tile->m_top_margin == 1 &&
           bottom_right_tile->m_left_margin == 1);
    assert(m_bottom_margin == 1 && m_right_margin == 1);

    gsl_matrix_set(m_matrix, m_height + m_top_margin, m_width + m_left_margin,
                   gsl_matrix_get(bottom_right_tile->m_matrix, 1, 1));
  }
}

} // namespace praster