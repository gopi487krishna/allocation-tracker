#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace AllocationTracer
{

    /**
     * @brief Stores the allocation information.
     * 
     */
    struct TracerRecord
    {
        std::string line_no;
        std::string filename;
        std::string directory;
    };

    /**
     * @brief Writes a tracer Record into the tracer database
     * 
     * @param tracer_record record information
     * @param tracer_db Database to write the record into
     */
    void writeTracerRecord(const TracerRecord& tracer_record, SQLite::Database& tracer_db){
        SQLite::Statement insert_statement(tracer_db,"INSERT INTO AllocTracer VALUES(null,?,?,?");
        insert_statement.bind(1,tracer_record.line_no);
        insert_statement.bind(2,tracer_record.filename);
        insert_statement.bind(3,tracer_record.directory);

        insert_statement.exec();
    }

    /**
     * @brief Inserts the module with tracers at allocations and updates the tracer database
     * 
     */
    struct Tracer
    {
        
        /**
         * @brief Construct a new Tracer object
         * 
         * @param mod Module to run the tracer through
         * @param tracerdb Database to write tracer information to
         */
        Tracer(llvm::Module& mod, SQLite::Database& tracer_db);

        /**
         * @brief Find allocations inside the functions, adds tracer and updates database
         * 
         * @param function Function to process
         */
        void processFunction(llvm::Function &function);

    private:
        llvm::Module &m_module;
        SQLite::Database &m_tracerdb;
    };

    Tracer::Tracer(llvm::Module& mod, SQLite::Database& tracer_db):m_module(mod),m_tracerdb(tracer_db){}
    void Tracer::processFunction(llvm::Function& function){

    }
}