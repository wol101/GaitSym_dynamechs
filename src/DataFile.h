// DataFile.h - utility class to read in various sorts of data files

#ifndef DataFile_h
#define DataFile_h

class DataFile
{
  public:
    DataFile();
    ~DataFile();

    // initialise by reading the file
    // this routine allocates a buffer and reads in the whole file
    bool ReadFile(const char * const name);
    // write the data out to a file
    bool WriteFile(const char * const name, bool binary = false);
    
    // retrieve basic types
    // the file is searched for the parameter and then the next token is read
    bool RetrieveParameter(const char * const param,
                           int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param,
                           long *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param,
                           unsigned int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param,
                           unsigned long *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, 
        double *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, 
        bool *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, 
        char *val, int size, bool searchFromStart = true);
    bool RetrieveQuotedStringParameter(const char * const param, 
        char *val, int size, bool searchFromStart = true);
    
    // ranged functions
    // the file is searched for the parameter and then the next token is read
    bool RetrieveRangedParameter(const char * const param, 
        double *val, bool searchFromStart = true);
    void SetRangeControl(double r) { m_RangeControl = r; };
    
    // line reading functions
    // optional comment character and can ignore empty lines
    bool ReadNextLine(char *line, int size, bool ignoreEmpty,
          char commentChar = 0); 
    
    // retrieve arrays
    // the file is searched for the parameter and then the next token is read
    bool RetrieveParameter(const char * const param, int n,
        int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, int n,
        double *val, bool searchFromStart = true);
    bool RetrieveRangedParameter(const char * const param, int n,
        double *val, bool searchFromStart = true);
    
    // utility settings
    void SetExitOnError(bool flag) { m_ExitOnErrorFlag = flag; };
    bool GetExitOnError() { return m_ExitOnErrorFlag; };
    char * GetRawData() { return m_FileData; };
    void SetRawData(const char *string);
    void ResetIndex() { m_Index = m_FileData; };
    int GetSize() { return m_Size; };
    void ClearData();

    
    // probably mostly for internal use
    // read the next ASCII token from the current index and bump index
    bool FindParameter(const char * const param, bool searchFromStart = true);
    bool ReadNext(int *val);
    bool ReadNext(double *val);
    bool ReadNext(char *val, int size);
    bool ReadNextQuotedString(char *val, int size);
    bool ReadNextLine(char *line, int size);
    bool ReadNextRanged(double *val);
    
    // handy statics
    static bool EndOfLineTest(char **p);
    static int CountTokens(char *string);
    static int ReturnTokens(char *string, char *ptrs[], int size);
    static void Strip(char *str);
    
    // binary file operators
    // read the next binary value from current index and bump index
    bool ReadNextBinary(int *val);
    bool ReadNextBinary(float *val);
    bool ReadNextBinary(double *val);
    bool ReadNextBinary(char *val);
    bool ReadNextBinary(bool *val);
    bool ReadNextBinary(int *val, int n);
    bool ReadNextBinary(float *val, int n);
    bool ReadNextBinary(double *val, int n);
    bool ReadNextBinary(char *val, int n);
    bool ReadNextBinary(bool *val, int n);

    // file writing operators
    bool WriteParameter(const char * const param, int val);
    bool WriteParameter(const char * const param, long val);
    bool WriteParameter(const char * const param, double val);
    bool WriteParameter(const char * const param, bool val);
    bool WriteParameter(const char * const param, const char * const val);
    bool WriteQuotedStringParameter(const char * const param, const char * const val);
    bool WriteParameter(const char * const param, int n, int *val);
    bool WriteParameter(const char * const param, int n, long *val);
    bool WriteParameter(const char * const param, int n, double *val);
    bool WriteParameter(const char * const param, int n, bool *val);
    bool WriteNext(int val, char after = 0);
    bool WriteNext(long val, char after = 0);
    bool WriteNext(double val, char after = 0);
    bool WriteNext(bool val, char after = 0);
    bool WriteNext(const char * const val, char after = 0);
    bool WriteNextQuotedString(const char * const val, char after = 0);

  protected:
    char * m_FileData;
    char * m_Index;
    bool m_ExitOnErrorFlag;
    double m_RangeControl;
    long m_Size;
};

#endif 

