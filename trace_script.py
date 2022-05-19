# traces = gdb.parse_and_eval("mem_trace::record")
# attributes = dir(traces.type)
# print(attributes)
# print(traces.type.size)

from inspect import trace
import sqlite3

class Trace(gdb.Command):
    def __init__(self):
        gdb.Command.__init__(self,"trace_adr",gdb.COMMAND_DATA,gdb.COMPLETE_SYMBOL,True)


    def invoke(self,arg,from_tty):
        # Fetch the address that we want to work with
        args = gdb.string_to_argv(arg)
        if len(args) <1 :
            print("Supply address as the input to the command")
            return
        address = int(args[0],16)
        trace_db_path = args[1]


        # Get the number of records present in the program
        size =self.get_record_size()
        

        #Get the traces
        traces = gdb.parse_and_eval("mem_trace::record")

        for index in range(0,100):
            trace = traces[index]
            if self.in_trace(address,trace):
                uid = self.get_uid(trace)
                trace_record = self.get_trace_record(uid,trace_db_path)
                self.printRecord(trace_record,trace)


 
    def in_trace(self,address,trace):
        start_address = int(trace['m_startaddress'])
        end_address = int(trace['m_endaddress'])
        if address >= start_address and address <= end_address:
            return True
        return False

    def get_uid(self,trace):
        return int(trace['m_uid'])

    def get_trace_record(self,uid,trace_db_path):
        conn = sqlite3.connect(trace_db_path)
        trace_record = conn.execute("SELECT * FROM AllocTracer WHERE UID=?",(uid,))
        
        return trace_record.fetchone()

    def get_record_size(self):
        return int(gdb.parse_and_eval("sizeof(mem_trace::record)/sizeof(*mem_trace::record)"))

    def printRecord(self, trace_record,trace):
        print("UID:\t" + str(trace_record[0]))
        print("Line Number:\t" + str(trace_record[1]))
        print("Filename:\t" + trace_record[2])
        print("Directory:\t" + trace_record[3])
        print("StartAddress:\t" + str(trace['m_startaddress']))
        print("EndAddress:\t" + str(trace['m_endaddress']))

Trace()