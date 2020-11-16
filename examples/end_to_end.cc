#include <assert.h>

#include <mpi.h>

#include "hermes.h"
#include "bucket.h"

namespace hapi = hermes::api;

int main(int argc, char **argv) {
  // Initialize MPI (MPI_THREAD_MULTIPLE is required)
  int mpi_threads_provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_threads_provided);
  if (mpi_threads_provided < MPI_THREAD_MULTIPLE) {
    fprintf(stderr, "Didn't receive appropriate MPI threading specification\n");
    return 1;
  }

  // Pass a hermes configuration file as the first argument
  if (argc != 2) {
    fprintf(stderr, "Usage: %s config_file\n", argv[0]);
    return 1;
  }

  char *config_file = argv[1];

  // Initialize Hermes on every rank
  std::shared_ptr<hapi::Hermes> hermes = hapi::InitHermes(config_file);

  if (hermes->IsApplicationCore()) {
    // Application code

    // Get this process's rank
    int app_rank = hermes->GetProcessRank();
    // The the total number of application ranks (not counting the Hermes rank)
    int app_size = hermes->GetNumProcesses();

    // Create a default context to pass to API calls
    hapi::Context ctx;
    // Create a bucket name unique to this rank
    std::string bucket_name = "example" + std::to_string(app_rank);
    // Each rank creates is own bucket
    hapi::Bucket bucket(bucket_name, hermes, ctx);

    hapi::PlacementPolicy strategies[] = {
      hapi::PlacementPolicy::kRandom,
      hapi::PlacementPolicy::kRoundRobin,
      hapi::PlacementPolicy::kMinimizeIoTime,
    };

    const char *strategy_names[] = {
      "Random",
      "RoundRobin",
      "MinimizeIoTime",
    };

    // fixed sized
    // random size

    for (size_t strat_index = 0;
         strat_index < sizeof(strategies) / sizeof(hapi::PlacementPolicy);
         ++strat_index) {
      // Create an API context for this Put, and set the policy
      hapi::Context put_ctx;
      put_ctx.policy = strategies[strat_index];

      size_t num_blobs = (app_rank + 1) * 5;
      std::vector<std::vector<int>> blob_batch(num_blobs);
      std::vector<std::string> blob_names(num_blobs);
      for (size_t i = 0; i < num_blobs; ++i) {
        // Create a name and the data for this blob
        blob_names[i] = ("blob_" + std::string(strategy_names[strat_index]) +
                         std::to_string(app_rank) + "_" + std::to_string(i));
        blob_batch[i].resize(MEGABYTES(4) / sizeof(int));
        for (size_t j = 0; j < blob_batch[i].size(); ++j) {
          blob_batch[i][j] = app_rank;
        }
      }

      // Buffer the blob in the hierarchy according to the desired policy
      Status status = bucket.Put(blob_names, blob_batch, put_ctx);

      //  Print summary + visual of buffer allocation (pick a core or two)
      //    Timings (min/max rank) "DPE solution," allocation, transfer
      //    Compare the estimated placement time/throughput w/ actual
      //    Which buffers were allocated
    }

    // Wait for all application ranks to arrive here
    hermes->AppBarrier();

    // Destroy the bucket, which deletes all the blobs is contains
    bucket.Destroy(ctx);

  } else {
    // Hermes core rank. No user code here.
  }

  // Shutdown Hermes
  hermes->Finalize();

  // Shutdown MPI
  MPI_Finalize();

  return 0;
}
