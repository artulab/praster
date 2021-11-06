#ifndef PRASTER_STRIPE_H
#define PRASTER_STRIPE_H

#include "tile.h"

#include <functional>
#include <memory>
#include <vector>

namespace praster {

class stripe {
public:
  inline int get_width() const noexcept { return m_width; }

  inline int get_height() const noexcept { return m_height; }

  inline int get_x() const noexcept { return 0; }

  inline int get_y() const noexcept { return m_y; }

  inline int get_top_margin() const noexcept { return m_top_margin; }

  inline int get_bottom_margin() const noexcept { return m_bottom_margin; }

  inline int get_tile_width() const noexcept { return m_tile_width; }

  inline int get_tile_height() const noexcept { return m_tile_height; }

  inline int get_stripe_number() const noexcept { return m_stripe_number; }

  inline const std::vector<tile> &get_tiles() const noexcept { return m_tiles; }

  void update_borders(stripe *upper_stripe, stripe *lower_stripe);

  static stripe
  build(int partition_number, int y, int height, int ds_width, int ds_height,
        double nodata, int tile_width, int tile_height,
        std::function<void(int x, int y, int width, int height, double *buffer)>
            callback);

private:
  int m_stripe_number;
  int m_y;
  int m_width;
  int m_height;
  int m_top_margin;
  int m_bottom_margin;
  double m_nodata;
  int m_tile_width;
  int m_tile_height;
  int m_tile_xsize{0};
  int m_tile_ysize{0};
  std::vector<tile> m_tiles;
  std::vector<int> m_top_margin_indices;
  std::vector<int> m_bottom_margin_indices;

  stripe(
      int stripe_number, int y, int width, int height, int top_margin,
      int bottom_margin, double nodata, int tile_width, int tile_height,
      std::function<void(int x, int y, int width, int height, double *buffer)>
          callback);

  inline int compute_tile_index(int y_id, int x_id) const noexcept {
    return m_tile_xsize * y_id + x_id;
  }
};

}; // namespace praster

#endif