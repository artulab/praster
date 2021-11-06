#ifndef PRASTER_TILE_H
#define PRASTER_TILE_H

#include <cassert>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <gsl/gsl_matrix.h>

namespace praster {

class tile {
public:
  tile(int x, int y, int width, int height, int left_margin, int right_margin,
       int top_margin, int bottom_margin, double nodata);

  tile(int x, int y, int width, int height, int left_margin, int right_margin,
       int top_margin, int bottom_margin, double nodata,
       std::function<void(int x, int y, int width, int height, double *buffer)>
           read_callback);

  virtual ~tile();

  tile(const tile &t) = delete;
  tile &operator=(tile &other) = delete;

  tile(tile &&other) noexcept;
  tile &operator=(tile &&other) noexcept;

  static inline std::pair<int, int> construct_index(const int y,
                                                    const int x) noexcept {
    return std::make_pair(y, x);
  }

  static inline int
  destruct_index_y(const std::pair<int, int> &index) noexcept {
    return index.first;
  }

  static inline int
  destruct_index_x(const std::pair<int, int> &index) noexcept {
    return index.second;
  }

  void foreach (std::function<void(const std::pair<int, int> &index)> callback);
  void foreach (std::function<void(const int y, const int x)> callback);
  void foreach (std::function<void(double val)> callback);

  void transform(std::function<double(double val)> callback);
  void transform(
      std::function<double(const int y, const int x, double val)> callback);
  void
  transform(std::function<double(const std::pair<int, int> &index, double val)>
                callback);

  std::vector<double> to_vector(bool with_border);

  void update_borders(tile *top_left_tile, tile *top_tile, tile *top_right_tile,
                      tile *left_tile, tile *right_tile, tile *bottom_left_tile,
                      tile *bottom_tile, tile *bottom_right_tile);

  inline int get_x() const noexcept { return m_x; }

  inline int get_y() const noexcept { return m_y; }

  inline int get_width() const noexcept { return m_width; }

  inline int get_height() const noexcept { return m_height; }

  inline int get_left_margin() const noexcept { return m_left_margin; }

  inline int get_right_margin() const noexcept { return m_right_margin; }

  inline int get_top_margin() const noexcept { return m_top_margin; }

  inline int get_bottom_margin() const noexcept { return m_bottom_margin; }

  inline int get_nodata() const noexcept { return m_nodata; }

  inline bool has_buffer() const noexcept { return m_matrix != nullptr; }

  inline double get_value(const std::pair<int, int> &index) noexcept {
    return gsl_matrix_get(m_matrix, index.first + m_top_margin,
                          index.second + m_left_margin);
  }

  inline double get_value(const int y, const int x) noexcept {
    return gsl_matrix_get(m_matrix, y + m_top_margin, x + m_left_margin);
  }

  inline void set_value(const std::pair<int, int> &index, double val) noexcept {
    gsl_matrix_set(m_matrix, index.first + m_top_margin,
                   index.second + m_left_margin, val);
  }

  inline void set_value(const int y, const int x, double val) noexcept {
    gsl_matrix_set(m_matrix, y + m_top_margin, x + m_left_margin, val);
  }

  inline std::array<double, 8> get_neighbours(const int y,
                                              const int x) noexcept {

    // Indices of the returned neighbour array:
    //
    //          +---+---+---+
    //          | 0 | 1 | 2 |
    //          +---+---+---+
    //          | 7 |   | 3 |
    //          +---+---+---+
    //          | 6 | 5 | 4 |
    //          +---+---+---+
    //

    return {
        (m_top_margin != 0 && m_left_margin != 0)
            ? gsl_matrix_get(m_matrix, y - 1 + m_top_margin,
                             x - 1 + m_left_margin)
            : m_nodata,
        (m_top_margin != 0)
            ? gsl_matrix_get(m_matrix, y - 1 + m_top_margin, x + m_left_margin)
            : m_nodata,
        (m_top_margin != 0 && m_right_margin != 0)
            ? gsl_matrix_get(m_matrix, y - 1 + m_top_margin,
                             x + 1 + m_left_margin)
            : m_nodata,
        (m_right_margin != 0)
            ? gsl_matrix_get(m_matrix, y + m_top_margin, x + 1 + m_left_margin)
            : m_nodata,
        (m_bottom_margin != 0 && m_right_margin != 0)
            ? gsl_matrix_get(m_matrix, y + 1 + m_top_margin,
                             x + 1 + m_left_margin)
            : m_nodata,
        (m_bottom_margin != 0)
            ? gsl_matrix_get(m_matrix, y + 1 + m_top_margin, x + m_left_margin)
            : m_nodata,
        (m_bottom_margin != 0 && m_left_margin != 0)
            ? gsl_matrix_get(m_matrix, y + 1 + m_top_margin,
                             x - 1 + m_left_margin)
            : m_nodata,
        (m_left_margin != 0)
            ? gsl_matrix_get(m_matrix, y + m_top_margin, x - 1 + m_left_margin)
            : m_nodata};
  }

  inline std::array<double, 8>
  get_neighbours(const std::pair<int, int> &index) noexcept {

    return get_neighbours(index.first, index.second);
  }

private:
  gsl_matrix *m_matrix{nullptr};
  int m_x{0};
  int m_y{0};
  int m_width{0};
  int m_height{0};
  int m_left_margin{0};
  int m_right_margin{0};
  int m_top_margin{0};
  int m_bottom_margin{0};
  double m_nodata{std::numeric_limits<double>::min()};
};

} // namespace praster

#endif
