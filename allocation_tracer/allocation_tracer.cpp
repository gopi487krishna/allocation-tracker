#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/ADT/StringRef.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include "allocation_tracer.hpp"

namespace
{
   using namespace llvm;
  struct AllocationTrackerPass : public ModulePass
  {
    static char ID;
    AllocationTrackerPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M)
    {
      // Open or Create SQLDB for writing record data
      SQLite::Database tracer_db("/home/cooldev/tracerdb.tdf",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
      std::string create_table_statement = \
      "CREATE TABLE IF NOT EXISTS AllocTracer (UID INTEGER AUTOINCREMENT PRIMARY KEY, LINE_NUM INTEGER, FILENAME TEXT, DIRECTORY TEXT );";
      tracer_db.exec(create_table_statement);

      //Tracer
      AllocationTracer::Tracer alloc_tracer(M,tracer_db);
      // Transformation Made
      return true;
    }
  };
}
char AllocationTrackerPass::ID = 0;


// Register the Module Pass as early as possible
static RegisterStandardPasses Z(
    PassManagerBuilder::EP_ModuleOptimizerEarly,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM)
    { PM.add(new AllocationTrackerPass()); });
