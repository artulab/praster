#include <iostream>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include <gdal.h>
#include <gdal_priv.h>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include "executor.h"

int main(int argc, char *argv[]) {


  using namespace praster;

  executor executor(5);

  executor.broadcast(
      [](executor::context c) {
        boost::this_thread::sleep_for(
            boost::chrono::milliseconds(c.task_num * 10));
        std::cout << c.task_num << " " << boost::this_thread::get_id()
                  << std::endl;
      },
      false);

  executor.broadcast(
      [](executor::context c) {
        boost::this_thread::sleep_for(
            boost::chrono::milliseconds(c.task_num * 10 + 100));
        std::cout << c.task_num << " hhh " << boost::this_thread::get_id()
                  << std::endl;
      },
      true);

  executor.submit(
      [](executor::context c) { std::cout << "heyoo" << std::endl; });

  GDALDataset *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *)GDALOpen(
      "/home/artu/projects/praster/test/data/raster-small.txt", GA_ReadOnly);
  if (poDataset == NULL) {
    std::cerr << "failed to open the file" << std::endl;
    return -1;
  }

  GDALRasterBand *poBand;

  poBand = poDataset->GetRasterBand(1);
  size_t nXSize = poBand->GetXSize();
  size_t nYSize = poBand->GetYSize();

  gsl_matrix *m = gsl_matrix_alloc(nYSize, nXSize);

  CPLErr err = poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, m->data, nXSize,
                                nYSize, GDT_Float64, 0, 0);

  if (err != CE_None) {
    std::cerr << "failed to read the file" << std::endl;
    return -1;
  }

  for (size_t i = 0; i < nYSize; ++i) {
    for (size_t j = 0; j < nXSize; ++j) {
      std::cout << gsl_matrix_get(m, i, j) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "column 0" << std::endl;

  gsl_vector_view column1 = gsl_matrix_column(m, 0);

  for (size_t j = 0; j < nYSize; ++j) {
    std::cout << gsl_vector_get(&column1.vector, j) << " ";
  }
  std::cout << std::endl;

  gsl_matrix_free(m);
}