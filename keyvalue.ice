module DSF {
    enum ValueType {
        Decimal,
        Integer,
        Bool,
        Text
    };
    class DataUnit {
        string strName;                 // the name of the measurement
        long lTime;                     // the timestamp of the measurement (ms) 

        ValueType   eType;                 // the type which determines which field is set
        optional(1) double dValue;         // Required if type is "Decimal"
        optional(2) long lValue;           // Required if type is "Integer"
        optional(3) bool bValue;           // Required if type is "Bool"
        optional(4) string strValue;       // Required if type is "Text"
    };
    sequence<DataUnit> DataUnitSeq;

    interface DataReceiver {
        void sendData(DataUnitSeq dataSeq);
    };
};