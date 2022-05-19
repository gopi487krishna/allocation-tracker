#include <stdlib.h>
#include "someother.h"
void mem_trace(long long uid, void* start_address, long long bytes_allocated){

    struct trace_rec{
        long long m_uid;
        void* m_startaddress;
        void* m_endaddress;
    };

    static struct trace_rec record[100];
    static int current_pos = 0;

    record[current_pos].m_uid = uid;
    record[current_pos].m_startaddress = start_address;
    record[current_pos].m_endaddress = start_address+bytes_allocated;

    current_pos = (current_pos+1)%100;
   

}

int main(){

    volatile float* my_value = q();

    x();
    p();
    return 0;

}