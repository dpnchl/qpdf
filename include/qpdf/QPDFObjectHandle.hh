// Copyright (c) 2005-2010 Jay Berkenbilt
//
// This file is part of qpdf.  This software may be distributed under
// the terms of version 2 of the Artistic License which may be found
// in the source distribution.  It is provided "as is" without express
// or implied warranty.

#ifndef __QPDFOBJECTHANDLE_HH__
#define __QPDFOBJECTHANDLE_HH__

#include <string>
#include <vector>
#include <set>
#include <map>

#include <qpdf/DLL.h>

#include <qpdf/PointerHolder.hh>
#include <qpdf/Buffer.hh>

#include <qpdf/QPDFObject.hh>

class Pipeline;
class QPDF;
class QPDF_Dictionary;
class QPDF_Array;

class QPDFObjectHandle
{
  public:
    QPDF_DLL
    QPDFObjectHandle();
    QPDF_DLL
    bool isInitialized() const;

    // Exactly one of these will return true for any object.
    QPDF_DLL
    bool isBool();
    QPDF_DLL
    bool isNull();
    QPDF_DLL
    bool isInteger();
    QPDF_DLL
    bool isReal();
    QPDF_DLL
    bool isName();
    QPDF_DLL
    bool isString();
    QPDF_DLL
    bool isArray();
    QPDF_DLL
    bool isDictionary();
    QPDF_DLL
    bool isStream();

    // This returns true in addition to the query for the specific
    // type for indirect objects.
    QPDF_DLL
    bool isIndirect();

    // True for everything except array, dictionary, and stream
    QPDF_DLL
    bool isScalar();

    // Public factory methods

    QPDF_DLL
    static QPDFObjectHandle newNull();
    QPDF_DLL
    static QPDFObjectHandle newBool(bool value);
    QPDF_DLL
    static QPDFObjectHandle newInteger(int value);
    QPDF_DLL
    static QPDFObjectHandle newReal(std::string const& value);
    QPDF_DLL
    static QPDFObjectHandle newName(std::string const& name);
    QPDF_DLL
    static QPDFObjectHandle newString(std::string const& str);
    QPDF_DLL
    static QPDFObjectHandle newArray(
	std::vector<QPDFObjectHandle> const& items);
    QPDF_DLL
    static QPDFObjectHandle newDictionary(
	std::map<std::string, QPDFObjectHandle> const& items);

    // Accessor methods.  If an accessor method that is valid for only
    // a particular object type is called on an object of the wrong
    // type, an exception is thrown.

    // Methods for bool objects
    QPDF_DLL
    bool getBoolValue();

    // Methods for integer objects
    QPDF_DLL
    int getIntValue();

    // Methods for real objects
    QPDF_DLL
    std::string getRealValue();

    // Methods that work for both integer and real objects
    QPDF_DLL
    bool isNumber();
    QPDF_DLL
    double getNumericValue();

    // Methods for name objects; see also name and array objects
    QPDF_DLL
    std::string getName();

    // Methods for string objects
    QPDF_DLL
    std::string getStringValue();
    QPDF_DLL
    std::string getUTF8Value();

    // Methods for array objects; see also name and array objects
    QPDF_DLL
    int getArrayNItems();
    QPDF_DLL
    QPDFObjectHandle getArrayItem(int n);

    // Methods for dictionary objects
    QPDF_DLL
    bool hasKey(std::string const&);
    QPDF_DLL
    QPDFObjectHandle getKey(std::string const&);
    QPDF_DLL
    std::set<std::string> getKeys();

    // Methods for name and array objects
    QPDF_DLL
    bool isOrHasName(std::string const&);

    // Mutator methods.  Use with caution.

    // Recursively copy this object, making it direct.  Throws an
    // exception if a loop is detected or any sub-object is a stream.
    QPDF_DLL
    void makeDirect();

    // Mutator methods for array objects
    QPDF_DLL
    void setArrayItem(int, QPDFObjectHandle const&);

    // Mutator methods for dictionary objects

    // Replace value of key, adding it if it does not exist
    QPDF_DLL
    void replaceKey(std::string const& key, QPDFObjectHandle const&);
    // Remove key, doing nothing if key does not exist
    QPDF_DLL
    void removeKey(std::string const& key);
    // If the object is null, remove the key.  Otherwise, replace it.
    QPDF_DLL
    void replaceOrRemoveKey(std::string const& key, QPDFObjectHandle);

    // Methods for stream objects
    QPDF_DLL
    QPDFObjectHandle getDict();

    // Returns filtered (uncompressed) stream data.  Throws an
    // exception if the stream is filtered and we can't decode it.
    QPDF_DLL
    PointerHolder<Buffer> getStreamData();

    // Write stream data through the given pipeline.  A null pipeline
    // value may be used if all you want to do is determine whether a
    // stream is filterable.  If filter is false, write raw stream
    // data and return false.  If filter is true, then attempt to
    // apply all the decoding filters to the stream data.  If we are
    // successful, return true.  Otherwise, return false and write raw
    // data.  If filtering is requested and successfully performed,
    // then the normalize and compress flags are used to determine
    // whether stream data should be normalized and compressed.  In
    // all cases, if this function returns false, raw data has been
    // written.  If it returns true, then any requested filtering has
    // been performed.  Note that if the original stream data has no
    // filters applied to it, the return value will be equal to the
    // value of the filter parameter.  Callers may use the return
    // value of this function to determine whether or not the /Filter
    // and /DecodeParms keys in the stream dictionary should be
    // replaced if writing a new stream object.
    QPDF_DLL
    bool pipeStreamData(Pipeline*, bool filter,
			bool normalize, bool compress);

    // Replace this stream's stream data with the given data buffer,
    // and replace the /Filter and /DecodeParms keys in the stream
    // dictionary with the given values.  (If either value is empty,
    // the corresponding key is removed.)  The stream's /Length key is
    // replaced with the length of the data buffer.  The stream is
    // interpreted as if the data read from the file, after any
    // decryption filters have been applied, is as presented.
    QPDF_DLL
    void replaceStreamData(PointerHolder<Buffer> data,
			   QPDFObjectHandle filter,
			   QPDFObjectHandle decode_parms);
    class StreamDataHandler
    {
      public:
	QPDF_DLL
	virtual ~StreamDataHandler()
	{
	}

	// See replaceStreamData(StreamDataHandler) below for a
	// description of how to override this function.
	virtual void
	replaceStreamData(Buffer const& in_data,
			  std::string const& in_filter,
			  std::string const& in_decode_parms,
			  bool filtered,
			  Buffer& out_data,
			  std::string& out_filter,
			  std::string& out_decode_parms,
			  bool& persist) = 0;
    };
    // Provide a hook for doing dynamic replacement of the stream's
    // data.  When the stream's data is accessed either with
    // pipeStreamData or with getStreamData, if the stream doesn't
    // already have replacement data, an attempt is first made to
    // filter the stream's original data.  If the attempt is
    // successful, the stream's filtered original data is passed to
    // the handler as in_data, and filtered is true.  If the original
    // data cannot be processed, then in_data is the original raw data
    // (after any decryption filters have been applied) and filtered
    // is false.  If the original input data has no filters applied,
    // the filtered is true.  This way, if filtered is true, the
    // caller knows that in_data contains the fully filtered data.
    // The handler then provides replacement data, /Filter, and
    // /DecodeParms (handled is in the simpler form of
    // replaceStreamData above).  If the persist argument is set to
    // true, then the replacement data is stored in the stream object
    // where it will be used on subsequent attempts to retrieve the
    // data (rather than calling the handler).  If persist is set to
    // false, then the data will be used that one time and not saved.
    // In that case, the handler will be invoked again if the stream
    // data is accessed another time.  Writing a handler that sets
    // persist to true essentially allows delaying the computation of
    // the stream data, while setting it to false reduces the amount
    // of memory that is used.
    QPDF_DLL
    void replaceStreamData(PointerHolder<StreamDataHandler> dh);

    // return 0 for direct objects
    QPDF_DLL
    int getObjectID() const;
    QPDF_DLL
    int getGeneration() const;

    QPDF_DLL
    std::string unparse();
    QPDF_DLL
    std::string unparseResolved();

    // Convenience routines for commonly performed functions

    // Throws an exception if this is not a Page object.  Returns an
    // empty map if there are no images or no resources.  This
    // function does not presently support inherited resources.  See
    // comment in the source for details.  Return value is a map from
    // XObject name to the image object, which is always a stream.
    QPDF_DLL
    std::map<std::string, QPDFObjectHandle> getPageImages();

    // Throws an exception if this is not a Page object.  Returns a
    // vector of stream objects representing the content streams for
    // the given page.  This routine allows the caller to not care
    // whether there are one or more than one content streams for a
    // page.
    QPDF_DLL
    std::vector<QPDFObjectHandle> getPageContents();

    // Initializers for objects.  This Factory class gives the QPDF
    // class specific permission to call factory methods without
    // making it a friend of the whole QPDFObjectHandle class.
    class Factory
    {
	friend class QPDF;
      private:
	static QPDFObjectHandle newIndirect(QPDF* qpdf,
					    int objid, int generation)
	{
	    return QPDFObjectHandle::newIndirect(qpdf, objid, generation);
	}
	// object must be dictionary object
	static QPDFObjectHandle newStream(
	    QPDF* qpdf, int objid, int generation,
	    QPDFObjectHandle stream_dict, off_t offset, int length)
	{
	    return QPDFObjectHandle::newStream(
		qpdf, objid, generation, stream_dict, offset, length);
	}
    };
    friend class Factory;

    // Accessor for raw underlying object -- only QPDF is allowed to
    // call this.
    class ObjAccessor
    {
	friend class QPDF;
      private:
	static PointerHolder<QPDFObject> getObject(QPDFObjectHandle& o)
	{
	    o.dereference();
	    return o.obj;
	}
    };
    friend class ObjAccessor;

    // Provide access to specific classes for recursive
    // reverseResolved().
    class ReleaseResolver
    {
	friend class QPDF_Dictionary;
	friend class QPDF_Array;
      private:
	static void releaseResolved(QPDFObjectHandle& o)
	{
	    o.releaseResolved();
	}
    };
    friend class ReleaseResolver;

  private:
    QPDFObjectHandle(QPDF*, int objid, int generation);
    QPDFObjectHandle(QPDFObject*);

    // Private object factory methods
    static QPDFObjectHandle newIndirect(QPDF*, int objid, int generation);
    static QPDFObjectHandle newStream(
	QPDF* qpdf, int objid, int generation,
	QPDFObjectHandle stream_dict, off_t offset, int length);

    void assertInitialized() const;
    void assertType(char const* type_name, bool istype);
    void assertPageObject();
    void dereference();
    void makeDirectInternal(std::set<int>& visited);
    void releaseResolved();

    bool initialized;

    QPDF* qpdf;			// 0 for direct object
    int objid;			// 0 for direct object
    int generation;
    PointerHolder<QPDFObject> obj;
};

#endif // __QPDFOBJECTHANDLE_HH__
