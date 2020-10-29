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
      // hapi::PlacementPolicy::kRandom,
      hapi::PlacementPolicy::kRoundRobin,
      hapi::PlacementPolicy::kMinimizeIoTime,
    };

    const char *strategy_names[] = {
      "RoundRobin",
      "MinimizeIoTime",
    };

#if 0
    switch (app_rank % 2) {
      case 0: {
        break;
      }
      case 1: {
        break;
      }
      default:
        break;
    }
#endif

    for (size_t i = 0;
         i < sizeof(strategies) / sizeof(hapi::PlacementPolicy);
         ++i) {
      // Create an API context for this Put, and set the policy
      hapi::Context put_ctx;
      put_ctx.policy = strategies[i];

      // Create a name and the data for this blob
      std::string blob_name = ("blob_" + std::string(strategy_names[i]) +
                               std::to_string(app_rank));
      std::vector<int> blob(MEGABYTES(4) / sizeof(int), app_rank);

      // Buffer the blob in the hierarchy according to the desired policy
      hapi::Status result = bucket.Put(blob_name, blob, put_ctx);

      if (result != 0) {
        // TODO(chogan): @errorhandling
        HERMES_NOT_IMPLEMENTED_YET;
      }

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
