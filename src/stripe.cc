#include "stripe.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace praster {

stripe::stripe(
    int stripe_number, int y, int width, int height, int top_margin,
    int bottom_margin, double nodata, int tile_width, int tile_height,
    std::function<void(int x, int y, int width, int height, double *buffer)>
        callback)
    : m_stripe_number(stripe_number), m_y(y), m_width(width), m_height(height),
      m_top_margin(top_margin), m_bottom_margin(bottom_margin),
      m_nodata(nodata), m_tile_width(tile_width), m_tile_height(tile_height) {

  m_tile_ysize =
      m_height / m_tile_height + (m_height % m_tile_height != 0 ? 1 : 0);
  m_tile_xsize = m_width / m_tile_width + (m_width % m_tile_width != 0 ? 1 : 0);

  int tile_index = 0;
  for (int y_id = 0; y_id < m_tile_ysize; ++y_id) {
    for (int x_id = 0; x_id < m_tile_xsize; ++x_id) {
      const int tile_left_margin = (x_id != 0 && m_tile_xsize != 1) ? 1 : 0;
      const int tile_right_margin =
          (x_id != m_tile_xsize - 1 && m_tile_xsize != 1) ? 1 : 0;

      const int tile_top_margin =
          (y_id != 0 && m_tile_ysize != 1) || (y_id == 0 && top_margin) ? 1 : 0;

      const int tile_bottom_margin =
          (y_id != m_tile_ysize - 1 && m_tile_ysize != 1) ||
                  (y_id == m_tile_ysize - 1 && bottom_margin)
              ? 1
              : 0;

      const int x = x_id * tile_width;
      const int y = m_y + y_id * tile_height;

      const int exact_tile_width =
          x_id == m_tile_xsize - 1 ? (m_width - (m_tile_xsize - 1) * tile_width)
                                   : tile_width;
      const int exact_tile_height =
          y_id == m_tile_ysize - 1
              ? (m_height - (m_tile_ysize - 1) * tile_height)
              : tile_height;
      tile t = tile(x, y, exact_tile_width, exact_tile_height, tile_left_margin,
                    tile_right_margin, tile_top_margin, tile_bottom_margin,
                    m_nodata, callback);

      m_tiles.push_back(std::move(t));

      if (y_id == 0) {
        m_top_margin_indices.push_back(tile_index);
      }

      if (y_id == m_tile_ysize - 1) {
        m_bottom_margin_indices.push_back(tile_index);
      }

      ++tile_index;
    }
  }
}

stripe stripe::build(
    int partition_number, int y, int height, int ds_width, int ds_height,
    double nodata, int tile_width, int tile_height,
    std::function<void(int x, int y, int width, int height, double *buffer)>
        callback) {
  if (tile_width > ds_width) {
    throw std::invalid_argument("stripe: m_tile_width > m_width");
  }

  if (tile_height > height) {
    throw std::invalid_argument("stripe: m_tile_height > m_height");
  }

  if (y + height > ds_height) {
    throw std::invalid_argument("stripe: m_y + m_height > ds_height");
  }

  int top_margin{1};
  if (y == 0) {
    top_margin = 0;
  }

  int bottom_margin{1};
  if (y + height == ds_height) {
    bottom_margin = 0;
  }

  return stripe(partition_number, y, ds_width, height, top_margin,
                bottom_margin, nodata, tile_width, tile_height, callback);
}

void stripe::update_borders(stripe *top_stripe, stripe *bottom_stripe) {

  // update borders located next to the top stripe

  if (top_stripe) {
    for (int i = 0; i < m_top_margin_indices.size(); ++i) {
      tile *center_tile = &m_tiles[m_top_margin_indices[i]];
      tile *top_tile =
          &top_stripe->m_tiles[top_stripe->m_bottom_margin_indices[i]];
      center_tile->update_borders(nullptr, top_tile, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, nullptr);
    }
  }

  // update borders located next to the bottom stripe

  if (bottom_stripe) {
    for (int i = 0; i < m_bottom_margin_indices.size(); ++i) {
      tile *center_tile = &m_tiles[m_bottom_margin_indices[i]];
      tile *bottom_tile =
          &bottom_stripe->m_tiles[bottom_stripe->m_top_margin_indices[i]];
      center_tile->update_borders(nullptr, nullptr, nullptr, nullptr, nullptr,
                                  nullptr, bottom_tile, nullptr);
    }
  }

  // lastly update inner borders

  tile *top_left_tile = nullptr;
  tile *top_tile = nullptr;
  tile *top_right_tile = nullptr;
  tile *left_tile = nullptr;
  tile *right_tile = nullptr;
  tile *bottom_left_tile = nullptr;
  tile *bottom_tile = nullptr;
  tile *bottom_right_tile = nullptr;
  tile *center_tile = nullptr;

  for (int y_id = 0; y_id < m_tile_ysize; ++y_id) {
    for (int x_id = 0; x_id < m_tile_xsize; ++x_id) {

      center_tile = &m_tiles[compute_tile_index(y_id, x_id)];

      if (y_id > 0 && x_id > 0) {
        top_left_tile = &m_tiles[compute_tile_index(y_id - 1, x_id - 1)];
      } else {
        top_left_tile = nullptr;
      }

      if (y_id > 0) {
        top_tile = &m_tiles[compute_tile_index(y_id - 1, x_id)];
      } else {
        top_tile = nullptr;
      }

      if (y_id > 0 && x_id < m_tile_xsize - 1) {
        top_right_tile = &m_tiles[compute_tile_index(y_id - 1, x_id + 1)];
      } else {
        top_right_tile = nullptr;
      }

      if (x_id > 0) {
        left_tile = &m_tiles[compute_tile_index(y_id, x_id - 1)];
      } else {
        left_tile = nullptr;
      }

      if (y_id > 0 && x_id < m_tile_xsize - 1) {
        right_tile = &m_tiles[compute_tile_index(y_id, x_id + 1)];
      } else {
        right_tile = nullptr;
      }

      if (x_id > 0 && y_id < m_tile_ysize - 1) {
        bottom_left_tile = &m_tiles[compute_tile_index(y_id + 1, x_id - 1)];
      } else {
        bottom_left_tile = nullptr;
      }

      if (y_id < m_tile_ysize - 1) {
        bottom_tile = &m_tiles[compute_tile_index(y_id + 1, x_id)];
      } else {
        bottom_tile = nullptr;
      }

      if (x_id < m_tile_xsize - 1 && y_id < m_tile_ysize - 1) {
        bottom_right_tile = &m_tiles[compute_tile_index(y_id + 1, x_id + 1)];
      } else {
        bottom_right_tile = nullptr;
      }

      center_tile->update_borders(top_left_tile, top_tile, top_right_tile,
                                  left_tile, right_tile, bottom_left_tile,
                                  bottom_tile, bottom_right_tile);
    }
  }
}

} // namespace praster