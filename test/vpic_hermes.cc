#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <chrono>

#include "mpi.h"
#include "hermes.h"

namespace hapi = hermes::api;

// Variables and dimensions
float *x, *y, *z;
float *px, *py, *pz;
int *id1, *id2;
int x_dim = 64;
int y_dim = 64;
int z_dim = 64;

// Uniform random number
inline double uniform_random_number() {
  return (((double)rand())/((double)(RAND_MAX)));
}

// Initialize particle data
void init_particles(int num_particles) {
  for (int i = 0; i < numparticles; i++) {
    id1[i] = i;
    id2[i] = i*2;
    x[i] = uniform_random_number()*x_dim;
    y[i] = uniform_random_number()*y_dim;
    z[i] = ((double)id1[i]/numparticles)*z_dim;
    px[i] = uniform_random_number()*x_dim;
    py[i] = uniform_random_number()*y_dim;
    pz[i] = ((double)id2[i]/numparticles)*z_dim;
  }
}

void master_printf(int rank, const *char msg) {
  if (rank == 0) {
    printf("%s", msg);
  }
}

void create_and_write_synthetic_data(hapi::Bucket &bucket, int rank) {

  hapi::Context ctx;
  bucket.Put("x", (u8 *)x, numparticles, ctx);
  bucket.Put("y", (u8 *)y, numparticles, ctx);
  bucket.Put("z", (u8 *)z, numparticles, ctx);
  bucket.Put("id1", (u8 *)id1, numparticles, ctx);
  bucket.Put("id2", (u8 *)id2, numparticles, ctx);
  bucket.Put("px", (u8 *)px, numparticles, ctx);
  bucket.Put("py", (u8 *)py, numparticles, ctx);
  bucket.Put("pz", (u8 *)pz, numparticles, ctx);
}

int main (int argc, char* argv[]) {

  char *file_name = argv[1];

  if (!file_name) {
    fprintf(stderr, "Enter the output file name as the first argument\n");
    exit(1);
  }

  const auto now = std::chrono::high_resolution_clock::now;

  MPI_Init(&argc, &argv);

  std::shared_ptr<hapi::Hermes> hermes = InitHermes();

  if (hermes->IsApplicationCore()) {
    int my_rank = hermes->GetProcessRank();

    if (argc == 3) {
      numparticles = (atoi(argv[2])) * 1024 * 1024;
    } else {
      numparticles = 8 * 1024 * 1024;
    }

    if (my_rank == 0) {
      printf("Number of particles: %ld \n", numparticles);
    }

    // TODO(chogan): Arena
    x = (float *)malloc(numparticles * sizeof(double));
    y = (float *)malloc(numparticles * sizeof(double));
    z = (float *)malloc(numparticles * sizeof(double));

    px = (float *)malloc(numparticles * sizeof(double));
    py = (float *)malloc(numparticles * sizeof(double));
    pz = (float *)malloc(numparticles * sizeof(double));

    id1 = (int *)malloc(numparticles * sizeof(int));
    id2 = (int *)malloc(numparticles * sizeof(int));

    init_particles();
    master_printf(my_rank, "Finished initializing particles \n");

    // h5part_int64_t alignf = 8*1024*1024;

    hermes->AppBarrier();

    auto start = now();
    // timer_on(0);

    long long total_particles;
    long long offset;

    MPI_Comm comm = hermes->GetAppComm();

    MPI_Allreduce(&numparticles, &total_particles, 1, MPI_LONG_LONG, MPI_SUM, comm);
    MPI_Scan(&numparticles, &offset, 1, MPI_LONG_LONG, MPI_SUM, comm);
    offset -= numparticles;

    hapi::Bucket bucket("vpic_bucket", hermes);

    hermes->AppBarrier();
    auto start1 = now();

    master_printf(rank, "Before writing particles \n");
    create_and_write_synthetic_data(my_rank);

    hermes->AppBarrier();
    auto end1 = now();
    master_printf(rank, "After writing particles \n");

    auto start2 = now();
    // H5Fflush(file_id, H5F_SCOPE_LOCAL);
    // TODO(chogan): Put this concept in the Context
    bucket.Persist();
    auto end2 = now();

    bucket.Destroy()

    free(x);
    free(y);
    free(z);
    free(px);
    free(py);
    free(pz);
    free(id1);
    free(id2);

    hermes->AppBarrier();

    auto end = now();

    if (my_rank == 0) {
      printf ("\nTiming results\n");
      double total_seconds_writing = std::chrono::duration<double>(end1 - start1).count();
      // timer_msg(1, "just writing data");
      double total_seconds_flushing = std::chrono::duration<double>(end2 - start2).count();
      // timer_msg(2, "flushing data");
      double total_seconds = std::chrono::duration<double>(end - start).count();
      // timer_msg(0, "opening, writing, flushing, and closing file");
      printf("\n");
    }
  } else {
    // Hermes core. No user code.
  }

  hermes->Finalize();

  MPI_Finalize();

  return 0;
}
