#ifndef DSF_ICE_INF_ICE
#define DSF_ICE_INF_ICE

#include <Ice/BuiltinSequences.ice>

module DSF {
    struct TagValue {
        string name;  // tag name, e.g. "STD::DINT[100]"
        string type;  // (Reserved fields) tag type, e.g. "DINT" "LREAL"  "BOOL" "STRING"
        int errCode;  // error , 0: success, other: error code
        Ice::ByteSeq value;  // tag value
    };

    sequence<TagValue> TagValueSeq; 
    struct ReadResult {
        int errCode;  // error , 0: success, other: error code
        TagValueSeq values; // tag value
    }; 

    interface DsfIceInf {
        bool queryRmStatus();  // read dsfstation rm_status . return：true: active; false : inactive
        ReadResult drRead(Ice::StringSeq names);  // batch read Data from dsfstation
        int drSave(TagValueSeq values); // batch Save Data to dsfstation ,return: 0: success, other: error code
    };
};


#endif