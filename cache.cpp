#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <cmath>
#include <unordered_map>
 
using namespace std; 


 long long clk;
 long long blocksize;
 long long l1size = 1024;
 long long l1assoc = 2;
 long long l2size = 65536;
 long long l2assoc = 8;

 std::vector<std::vector<long long>> l1cache; 
 std::vector<std::vector<long long>> l2cache;
 std::vector<std::vector<long long>> l1lru; 
 std::vector<std::vector<long long>> l2lru;
 std::vector<std::vector<long long>> l1dirty; 
 std::vector<std::vector<long long>> l2dirty;
 long long l1sets ;
 long long l2sets ;

 long long l1reads=0;
 long long l1readmisses=0;
 long long l2reads=0;
 long long l2readmisses=0;
 long long l1writebacks=0;
 
 long long l1writes=0;
 long long l1writemisses=0;
 long long l2writes=0;
 long long l2writemisses=0;
 long long l2writebacks=0;

 

void writeL2(long long address, string request){
   //   cout<<request <<" "<<address<<endl;
        l2writes++;
        const long long a = log(l2sets);
        //get index
        long long set_index = (address / blocksize) % l2sets;

        //get block offset
        long long block_offset = address % blocksize;;
        long long tag =  address / (blocksize * l2sets);
        

        // search for the block in the set
        long long block_index = -1;
        for (long long i = 0; i < l2assoc; i++) {
            if (l2cache[set_index][i] == tag) {
                block_index = i;
                break;
            }
        }
        // block hit
        if (block_index != -1) {
            // update LRU information
            l2lru[set_index][block_index] = clk++;
            l2dirty[set_index][block_index] = 1;
           // std::cout << "HIT\n";
        } 
        //block miss
        else {
            l2writemisses++;
            //search in main memory
              
            // search for the LRU block in the set
            long long lru_index = 0;
            long long lru_time = clk;
            for (long long i = 0; i < l2assoc; i++) {
                if (l2lru[set_index][i] < lru_time) {
                    lru_time = l2lru[set_index][i];
                    lru_index = i;
                }
            }

            //if block is dirty write back to next level
            if(l2dirty[set_index][lru_index]==1){
                l2writebacks++;
                long long t = l2cache[set_index][lru_index];
                l2dirty[set_index][lru_index]=0;
                long long add = t * (l2sets * blocksize) + set_index * blocksize + block_offset;
                //writeL2(add);
            }

            // replace the LRU block with the new block
            l2cache[set_index][lru_index] = tag;
            l2lru[set_index][lru_index] = clk++;
            //std::cout << "MISS\n";               

        }          
        

    return;
} 


void readL2(long long address, string request){
    //  cout<<request <<" "<<address<<endl;
        l2reads++;
        const long long a = log(l2sets);
        //get index
        long long set_index = (address / blocksize) % l2sets;

        //get block offset
        long long block_offset = address % blocksize;;
        long long tag =  address / (blocksize * l2sets);
        

        // search for the block in the set
        long long block_index = -1;
        for (long long i = 0; i < l2assoc; i++) {
            if (l2cache[set_index][i] == tag) {
                block_index = i;
                break;
            }
        }
        // block hit
        if (block_index != -1) {
            // update LRU information
            l2lru[set_index][block_index] = clk++;
            //l2writes++;
           // std::cout << "HIT\n";
        }    
         // block miss
        else {
            l2readmisses++;
            //search in main memory
              
            // search for the LRU block in the set
            long long lru_index = 0;
            long long lru_time = clk;
            for (long long i = 0; i < l2assoc; i++) {
                if (l2lru[set_index][i] < lru_time) {
                    lru_time = l2lru[set_index][i];
                    lru_index = i;
                }
            }

            //if block is dirty write back to next level
            if(l2dirty[set_index][lru_index]==1){
                l2writebacks++;
                long long t = l2cache[set_index][lru_index];
                l2dirty[set_index][lru_index]=0;
                long long add = t * (l2sets * blocksize) + set_index * blocksize + block_offset;
                //writeL2(add,request);
            }

             // when both write miss happend in l1 and l2
             //just write updated value to L1 and marked dirty also
        

            
            // replace the LRU block with the new block
            l2cache[set_index][lru_index] = tag;
            l2lru[set_index][lru_index] = clk++;
            
            //std::cout << "MISS\n";               

        }  
        
        return ; 
} 



void writeL1(long long address, string request){
     // cout<<request <<" "<<address<<endl;
        l1writes++;
        const long long a = log(l2sets);
        //get index
        long long set_index = (address / blocksize) % l1sets;

        //get block offset
        long long block_offset = address % blocksize;;
        long long tag =  address / (blocksize * l1sets);

        // search for the block in the set
        long long block_index = -1;
        for (long long i = 0; i < l1assoc; i++) {
            if (l1cache[set_index][i] == tag) {
                block_index = i;
                break;
            }
        }
        // block hit
        if (block_index != -1) {
            // update LRU information
            l1lru[set_index][block_index] = clk++;
            l1dirty[set_index][block_index] =1;
           // std::cout << "HIT\n";
        }  
        else {
             l1writemisses++;
            //   if(request=="r"){
                 readL2(address, request);
            // }else{
            //     writeL2(address,request);
            // }
            // search for the LRU block in the set
            long long lru_index = 0;
            long long lru_time = clk;
            for (long long i = 0; i < l1assoc; i++) {
                if (l1lru[set_index][i] < lru_time) {
                    lru_time = l1lru[set_index][i];
                    lru_index = i;
                }
            }

            //if block is dirty write back to next level
            if(l1dirty[set_index][lru_index]==1){
                l1writebacks++;
                long long t = l1cache[set_index][lru_index];
                l1dirty[set_index][lru_index]=0;
                long long add = t * (l1sets * blocksize) + set_index * blocksize + block_offset;
                writeL2(add,request);
                //when there both write misses in l1 and l2
                
             }

            l1dirty[set_index][lru_index]=1;
            // replace the LRU block with the new block
            l1cache[set_index][lru_index] = tag;
            l1lru[set_index][lru_index] = clk++;
            //std:: cout << "MISS\n";               

        }                     

    return;
}



void readL1(long long address, string request){
      //   cout<<request <<" "<<address<<endl;
        
        l1reads++;
        const long long a = log(l2sets);
        //get index
        long long set_index = (address / blocksize) % l1sets;

        //get block offset
        long long block_offset = address % blocksize;;
        long long tag =  address / (blocksize * l1sets);

        // search for the block in the set
        long long block_index = -1;
        for (long long i = 0; i < l1assoc; i++) {
            if (l1cache[set_index][i] == tag) {
                block_index = i;
                break;
            }
        }
        // block hit
        if (block_index != -1) {
            // update LRU information
            l1lru[set_index][block_index] = clk++;
           // std::cout << "HIT\n";
        }    
         // block miss
        else {
            l1readmisses++;
            // if(request=="r"){
                
            // }else{
            //     writeL2(address,request);
            // }
           
              
            // search for the LRU block in the set
            //search for smallest time block
            long long lru_index = 0;
            long long lru_time = clk;
            for (long long i = 0; i < l1assoc; i++) {
                if (l1lru[set_index][i] < lru_time) {
                    lru_time = l1lru[set_index][i];
                    lru_index = i;
                }
            }

            //if block is dirty write back to next level
            if(l1dirty[set_index][lru_index]==1){
                l1writebacks++;
                long long t = l1cache[set_index][lru_index];
                l1dirty[set_index][lru_index]=0;
                long long add = t * (l1sets * blocksize) + set_index * blocksize + block_offset;
                writeL2(add,request);
             }
            
             readL2(address, request);
            
            // replace the LRU block with the new block
            l1cache[set_index][lru_index] = tag;
            l1lru[set_index][lru_index] = clk++;
            //std:: cout << "MISS\n";               

        }  
        return ; 
}         





void fun(string s){

    //fun(s);
    const char *love= s.c_str();
    std::ifstream myfile(love);

   //std::ifstream myfile (argv[1]);
    std::string line;

    l1cache.resize(l1sets);
    l2cache.resize(l2sets);
    l1lru.resize(l1sets);
    l2lru.resize(l2sets);
    l1dirty.resize(l1sets);
    l2dirty.resize(l2sets);
    for(long long i=0;i<l1sets;i++){
        l1cache[i].resize(l1assoc,-1);
    }
    for(long long i=0;i<l2sets;i++){
        l2cache[i].resize(l2assoc,-1);
    }
    for(long long i=0;i<l1sets;i++){
        l1lru[i].resize(l1assoc,-1);
    }
    for(long long i=0;i<l2sets;i++){
        l2lru[i].resize(l2assoc,-1);
    }
    for(long long i=0;i<l1sets;i++){
        l1dirty[i].resize(l1assoc,0);
    }
    
            //cout<<"yes"<<endl;
    for(long long i=0;i<l2sets;i++){
        l2dirty[i].resize(l2assoc,0);
    }
  if (myfile.is_open())
  {
        while ( std::getline (myfile,line) )
        {  
            std::string rw ;
            std::string pc ="" ;
            rw = line[0];
            for(long long i=2;i<line.size();i++){
                pc+=line[i];
            }
            long long address = stoll(pc, nullptr, 16); 
            if(rw=="r"){
                readL1(address,rw);
            }
            if(rw=="w"){
                writeL1(address,rw);
            }
            
        
    }      
  }

  float a=(l1readmisses+l1writemisses);
  float b=(l1reads+l1writes);
  a=a/b;
  float c=(l2readmisses+l2writemisses);
  float d=(l2reads+l2writes);
  c=c/d;
  

//   cout<<"===== Simulation Results =====" <<endl;
//   cout<<"i. number of L1 reads:"<<"    "<<l1reads<<endl;
//   cout<<"ii. number of L1 read misses: "<<"    "<<l1readmisses<<endl;
//   cout<<"iii. number of L1 writes:"<<"    "<<l1writes<<endl;
//   cout<<"iv. number of L1 write misses:"<<"    "<<l1writemisses<<endl;
//   cout<<"v. L1 miss rate:  "<<"  "<<a<<endl;
//   cout<<"vi. number of writebacks from L1 memory: "<< l1writebacks<<endl;
 

//     cout<<"vii. number of L2 reads:"<<"    "<<l2reads<<endl;
//   cout<<"viii. number of L2 read misses: "<<"    "<<l2readmisses<<endl;
//   cout<<"ix. number of L2 writes:"<<"    "<<l2writes<<endl;
//   cout<<"x. number of L2 write misses:"<<"    "<<l2writemisses<<endl;
//   cout<<"xi. L2 miss rate:  "<<"  "<<c<<endl;
//   cout<<"xii. number of writebacks from L2 memory: "<< l2writebacks<<endl;

  long long total_access_time= (l1reads+l1writes)*1 + (l2reads+l2writes )*20 + (l2writebacks+l2writemisses+l2readmisses)*200;
  //cout<<"TOTAL ACCESS TIME : "<<total_access_time<<" ns"<<endl;
  cout<<total_access_time<<" ";


}



int main(int argc, char *argv[])
{
 
//  if (argc != 2)
// 	{
// 		std::cerr << "Required argument: file_name\n./main <file name>\n";
// 		return 0;
// 	}
   

   	string s="trace1.txt";
    
    vector<string> trace_arr={"trace1.txt","trace2.txt","trace3.txt","trace4.txt","trace5.txt","trace6.txt","trace7.txt","trace8.txt"};
    vector<long long> blocksize_arr= {8, 16, 32, 64, 128};
    vector<long long> l1size_arr={512, 1024, 2048,4096, 8192};
    vector<long long> l1assoc_arr={1, 2, 4, 8, 16};
    vector<long long> l2size_arr={16384, 32768, 65536,131072, 262144};
    vector<long long> l2assoc_arr={1, 2, 4, 8, 16};
  
    
    //blocksize
    for(int i=0; i<blocksize_arr.size(); i++){

        blocksize=blocksize_arr[i];
        l1size=1024;
        l1assoc=2;
        l2size=65536;
        l2assoc=8;
        l1sets = (long long) l1size / (blocksize * l1assoc);
        l2sets = (long long) l2size / (blocksize * l2assoc);
        
        cout<<blocksize<<" ";

        for(int j=0; j<trace_arr.size(); j++){
            s=trace_arr[j];    
            fun(s);
        }
        cout<<""<<endl;

    }
    
    cout<<"  "<<endl;

    
    //l1size
    for(int i=0; i<l1size_arr.size(); i++){

        blocksize=64;
        l1size=l1size_arr[i];
        l1assoc=2;
        l2size=65536;
        l2assoc=8;
        l1sets = (long long) l1size / (blocksize * l1assoc);
        l2sets = (long long) l2size / (blocksize * l2assoc);
        
        cout<<l1size<<" ";

        for(int j=0; j<trace_arr.size(); j++){
            s=trace_arr[j];    
            fun(s);
        }
        cout<<""<<endl;

    }
    

    
    cout<<"  "<<endl;
    //l1assoc
    for(int i=0; i<l1assoc_arr.size(); i++){

        blocksize=64;
        l1size=1024;
        l1assoc=l1assoc_arr[i];
        l2size=65536;
        l2assoc=8;
        l1sets = (long long) l1size / (blocksize * l1assoc);
        l2sets = (long long) l2size / (blocksize * l2assoc);
        
        cout<<l1assoc<<" ";

        for(int j=0; j<trace_arr.size(); j++){
            s=trace_arr[j];    
            fun(s);
        }
        cout<<""<<endl;

    }
    
    cout<<"  "<<endl;
    
    
    //l2size
    for(int i=0; i<l2size_arr.size(); i++){

        blocksize=64;
        l1size=1024;
        l1assoc=2;
        l2size=l2size_arr[i];
        l2assoc=8;
        l1sets = (long long) l1size / (blocksize * l1assoc);
        l2sets = (long long) l2size / (blocksize * l2assoc);
        
        cout<<l2size<<" ";

        for(int j=0; j<trace_arr.size(); j++){
            s=trace_arr[j];    
            fun(s);
        }
        cout<<""<<endl;

    }
    
    cout<<"  "<<endl;
    
    //l2assoc
    for(int i=0; i<l2assoc_arr.size(); i++){

        blocksize=64;
        l1size=1024;
        l1assoc=2;
        l2size=65536;
        l2assoc=l2assoc_arr[i];
        l1sets = (long long) l1size / (blocksize * l1assoc);
        l2sets = (long long) l2size / (blocksize * l2assoc);
        
        cout<<l2assoc<<" ";

        for(int j=0; j<trace_arr.size(); j++){
            s=trace_arr[j];    
            fun(s);
        }
        cout<<""<<endl;

    }
    


    

     


	
  return 0;
}  