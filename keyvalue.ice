module DSF {
    enum ValueType {
        Decimal,
        Integer,
        Boolean,
        Text
    };
    struct DataUnit {
        string strName;                 // the name of the measurement
        long lTime;                     // the timestamp of the measurement (ms)
 
        ValueType   eType;                 // the type which determines which field is set
        double dValue;         // Required if type is "Decimal"
        long lValue;           // Required if type is "Integer"
        bool bValue;           // Required if type is "Bool"
        string strValue;       // Required if type is "Text"
    };
    sequence<DataUnit> DataUnitSeq;
 
    interface DataReceiver {
        void sendData(DataUnitSeq dataSeq);
    };
};