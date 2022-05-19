#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>

#include <SQLiteCpp/SQLiteCpp.h>

namespace AllocationTrace
{

    /**
     * @brief Stores the allocation information.
     *
     */
    struct TraceRecord
    {
        std::uint32_t line_no;
        std::string filename;
        std::string directory;
    };

    /**
     * @brief Writes a tracer Record into the tracer database
     *
     * @param trace_record record information
     * @param trace_db Database to write the record into
     * @return Return row id (uid)
     */
    std::int64_t writeIntoDB(const TraceRecord &trace_record, SQLite::Database &trace_db)
    {
        try{
        SQLite::Statement insert_statement(trace_db, "INSERT INTO AllocTracer VALUES(null,?,?,?);");
        insert_statement.bind(1, trace_record.line_no);
        insert_statement.bind(2, trace_record.filename);
        insert_statement.bind(3, trace_record.directory);

        insert_statement.exec();
        }
        catch (SQLite::Exception& e){
            llvm::errs() << e.what();
        }
        return trace_db.getLastInsertRowid();
    }

    /**
     * @brief Inserts the module with tracers at allocations and updates the tracer database
     *
     */
    struct Trace
    {

        static void processFunction(llvm::Function &function, SQLite::Database &trace_db);
        static void processModule(llvm::Module &mod, SQLite::Database &trace_db);

    private:
        static void insertTraceAfter(llvm::CallInst *call_instruction, std::int64_t uid, llvm::Module &mod);
        static TraceRecord generateTraceRecord(llvm::CallInst *call_instruction);
        static llvm::FunctionCallee getMemTraceFunc(llvm::Module &mod);
    };

    void Trace::processFunction(llvm::Function &function, SQLite::Database &trace_db)
    {
        llvm::errs() << "Processing Function : " << function.getName() << '\n';
        for (auto instruction_it = llvm::inst_begin(function); instruction_it != llvm::inst_end(function); instruction_it++)
        {
            llvm::Instruction *instruction = &(*instruction_it);
            if (llvm::isa<llvm::CallInst>(instruction))
            {

                llvm::CallInst *call_instruction = (llvm::CallInst *)instruction;
                if (call_instruction->getCalledFunction()->getName() == "malloc")
                {
                    llvm::errs() << "Found malloc\n";
                    // Generate Record Information
                    TraceRecord trace_record = generateTraceRecord(call_instruction);
                    // Insert Record into database
                    std::int64_t uid = writeIntoDB(trace_record, trace_db);
                    // Insert tracer after the call instruction
                    insertTraceAfter(call_instruction, uid, *function.getParent());
                }
            }
        }
    }

    TraceRecord Trace::generateTraceRecord(llvm::CallInst *call_instruction)
    {
        TraceRecord trace_record;
        llvm::DILocation *loc = call_instruction->getDebugLoc();

        trace_record.line_no = loc->getLine();
        trace_record.filename = loc->getFilename().str();
        trace_record.directory = loc->getDirectory().str();

        return trace_record;
    }

    void Trace::insertTraceAfter(llvm::CallInst *call_instruction, std::int64_t uid, llvm::Module &mod)
    {
        // Get mem_trace function
        auto mem_trace_func = getMemTraceFunc(mod);

        // By default we set the insert point at the end of basic block(Assumes this call is the last instruction. What about return statements ?)
        llvm::IRBuilder<> builder(call_instruction->getParent());

        // If call is not last instruction then insert after call
        if (call_instruction->getNextNode() != nullptr)
        {
            // Insert just after kmalloc
            builder.SetInsertPoint(call_instruction->getNextNode());
        }

        // Prepare arguments for the function call
        llvm::Value *uid_arg = llvm::ConstantInt::get(llvm::Type::getInt64Ty(mod.getContext()), uid, false);
        llvm::Value *start_address_arg = call_instruction; // Return value of call is the start address
        llvm::Value *bytes_allocated_arg = call_instruction->getArgOperand(0);

        llvm::Value *start_address_i8_arg = builder.CreateBitCast(start_address_arg, llvm::Type::getInt8PtrTy(mod.getContext()));

        std::vector<llvm::Value *> function_args;
        function_args.push_back(uid_arg);
        function_args.push_back(start_address_i8_arg);
        function_args.push_back(bytes_allocated_arg);

        // Add the call instruction
        llvm::CallInst *mem_trace_call = builder.CreateCall(mem_trace_func, function_args);
    }

    llvm::FunctionCallee Trace::getMemTraceFunc(llvm::Module &mod)
    {

        auto *ret_type = llvm::Type::getVoidTy(mod.getContext());
        auto *uid_type = llvm::Type::getInt64Ty(mod.getContext());
        auto *start_address_type = llvm::Type::getInt8PtrTy(mod.getContext());
        auto *bytes_allocated_type = llvm::Type::getInt64Ty(mod.getContext());

        std::vector<llvm::Type *> argument_types;
        argument_types.push_back(uid_type);
        argument_types.push_back(start_address_type);
        argument_types.push_back(bytes_allocated_type);

        auto function_type = llvm::FunctionType::get(ret_type, argument_types, false);
        auto mem_trace_func = mod.getOrInsertFunction("mem_trace", function_type);

        return mem_trace_func;
    }

    void Trace::processModule(llvm::Module &mod, SQLite::Database &trace_db)
    {
        for (auto &function : mod)
        {
            processFunction(function, trace_db);
        }
    }

}