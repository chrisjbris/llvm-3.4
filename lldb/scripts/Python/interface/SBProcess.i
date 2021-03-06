//===-- SWIG Interface for SBProcess ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace lldb {

%feature("docstring",
"Represents the process associated with the target program.

SBProcess supports thread iteration. For example (from test/lldbutil.py),

# ==================================================
# Utility functions related to Threads and Processes
# ==================================================

def get_stopped_threads(process, reason):
    '''Returns the thread(s) with the specified stop reason in a list.

    The list can be empty if no such thread exists.
    '''
    threads = []
    for t in process:
        if t.GetStopReason() == reason:
            threads.append(t)
    return threads

...
"
) SBProcess;
class SBProcess
{
public:
    //------------------------------------------------------------------
    /// Broadcaster event bits definitions.
    //------------------------------------------------------------------
    enum
    {
        eBroadcastBitStateChanged   = (1 << 0),
        eBroadcastBitInterrupt      = (1 << 1),
        eBroadcastBitSTDOUT         = (1 << 2),
        eBroadcastBitSTDERR         = (1 << 3),
        eBroadcastBitProfileData    = (1 << 4)
    };

    SBProcess ();

    SBProcess (const lldb::SBProcess& rhs);

    ~SBProcess();

    static const char *
    GetBroadcasterClassName ();

    const char *
    GetPluginName ();
    
    const char *
    GetShortPluginName ();
    
    void
    Clear ();

    bool
    IsValid() const;

    lldb::SBTarget
    GetTarget() const;

    lldb::ByteOrder
    GetByteOrder() const;

    %feature("autodoc", "
    Writes data into the current process's stdin. API client specifies a Python
    string as the only argument.
    ") PutSTDIN;
    size_t
    PutSTDIN (const char *src, size_t src_len);

    %feature("autodoc", "
    Reads data from the current process's stdout stream. API client specifies
    the size of the buffer to read data into. It returns the byte buffer in a
    Python string.
    ") GetSTDOUT;
    size_t
    GetSTDOUT (char *dst, size_t dst_len) const;

    %feature("autodoc", "
    Reads data from the current process's stderr stream. API client specifies
    the size of the buffer to read data into. It returns the byte buffer in a
    Python string.
    ") GetSTDERR;
    size_t
    GetSTDERR (char *dst, size_t dst_len) const;

    size_t
    GetAsyncProfileData(char *dst, size_t dst_len) const;
    
    void
    ReportEventState (const lldb::SBEvent &event, FILE *out) const;

    void
    AppendEventStateReport (const lldb::SBEvent &event, lldb::SBCommandReturnObject &result);

    %feature("docstring", "
    //------------------------------------------------------------------
    /// Remote connection related functions. These will fail if the
    /// process is not in eStateConnected. They are intended for use
    /// when connecting to an externally managed debugserver instance.
    //------------------------------------------------------------------
    ") RemoteAttachToProcessWithID;
    bool
    RemoteAttachToProcessWithID (lldb::pid_t pid,
                                 lldb::SBError& error);
    
    %feature("docstring",
    "See SBTarget.Launch for argument description and usage."
    ) RemoteLaunch;
    bool
    RemoteLaunch (char const **argv,
                  char const **envp,
                  const char *stdin_path,
                  const char *stdout_path,
                  const char *stderr_path,
                  const char *working_directory,
                  uint32_t launch_flags,
                  bool stop_at_entry,
                  lldb::SBError& error);
    
    //------------------------------------------------------------------
    // Thread related functions
    //------------------------------------------------------------------
    uint32_t
    GetNumThreads ();

    %feature("autodoc", "
    Returns the INDEX'th thread from the list of current threads.  The index
    of a thread is only valid for the current stop.  For a persistent thread
    identifier use either the thread ID or the IndexID.  See help on SBThread
    for more details.
    ") GetThreadAtIndex;
    lldb::SBThread
    GetThreadAtIndex (size_t index);

    %feature("autodoc", "
    Returns the thread with the given thread ID.
    ") GetThreadByID;
    lldb::SBThread
    GetThreadByID (lldb::tid_t sb_thread_id);
    
    %feature("autodoc", "
    Returns the thread with the given thread IndexID.
    ") GetThreadByIndexID;
    lldb::SBThread
    GetThreadByIndexID (uint32_t index_id);

    %feature("autodoc", "
    Returns the currently selected thread.
    ") GetSelectedThread;
    lldb::SBThread
    GetSelectedThread () const;

    %feature("autodoc", "
    Lazily create a thread on demand through the current OperatingSystem plug-in, if the current OperatingSystem plug-in supports it.
    ") CreateOSPluginThread;
    lldb::SBThread
    CreateOSPluginThread (lldb::tid_t tid, lldb::addr_t context);

    bool
    SetSelectedThread (const lldb::SBThread &thread);

    bool
    SetSelectedThreadByID (lldb::tid_t tid);

    bool
    SetSelectedThreadByIndexID (uint32_t index_id);
    
    //------------------------------------------------------------------
    // Stepping related functions
    //------------------------------------------------------------------

    lldb::StateType
    GetState ();

    int
    GetExitStatus ();

    const char *
    GetExitDescription ();

    %feature("autodoc", "
    Returns the process ID of the process.
    ") GetProcessID;
    lldb::pid_t
    GetProcessID ();
    
    %feature("autodoc", "
    Returns an integer ID that is guaranteed to be unique across all process instances. This is not the process ID, just a unique integer for comparison and caching purposes.
    ") GetUniqueID;
    uint32_t
    GetUniqueID();

    uint32_t
    GetAddressByteSize() const;

    %feature("docstring", "
    Kills the process and shuts down all threads that were spawned to
    track and monitor process.
    ") Destroy;
    lldb::SBError
    Destroy ();

    lldb::SBError
    Continue ();

    lldb::SBError
    Stop ();

    %feature("docstring", "Same as Destroy(self).") Destroy;
    lldb::SBError
    Kill ();

    lldb::SBError
    Detach ();

    %feature("docstring", "Sends the process a unix signal.") Signal;
    lldb::SBError
    Signal (int signal);

    %feature("docstring", "
    Returns a stop id that will increase every time the process executes.  If
    include_expression_stops is true, then stops caused by expression evaluation
    will cause the returned value to increase, otherwise the counter returned will
    only increase when execution is continued explicitly by the user.  Note, the value
    will always increase, but may increase by more than one per stop.
    ") GetStopID;
    uint32_t
    GetStopID(bool include_expression_stops = false);
    
    void
    SendAsyncInterrupt();
    
    %feature("autodoc", "
    Reads memory from the current process's address space and removes any
    traps that may have been inserted into the memory. It returns the byte
    buffer in a Python string. Example:

    # Read 4 bytes from address 'addr' and assume error.Success() is True.
    content = process.ReadMemory(addr, 4, error)
    new_bytes = bytearray(content)
    ") ReadMemory;
    size_t
    ReadMemory (addr_t addr, void *buf, size_t size, lldb::SBError &error);

    %feature("autodoc", "
    Writes memory to the current process's address space and maintains any
    traps that might be present due to software breakpoints. Example:

    # Create a Python string from the byte array.
    new_value = str(bytes)
    result = process.WriteMemory(addr, new_value, error)
    if not error.Success() or result != len(bytes):
        print 'SBProcess.WriteMemory() failed!'
    ") WriteMemory;
    size_t
    WriteMemory (addr_t addr, const void *buf, size_t size, lldb::SBError &error);

    %feature("autodoc", "
    Reads a NULL terminated C string from the current process's address space.
    It returns a python string of the exact length, or truncates the string if
    the maximum character limit is reached. Example:
    
    # Read a C string of at most 256 bytes from address '0x1000' 
    error = lldb.SBError()
    cstring = process.ReadCStringFromMemory(0x1000, 256, error)
    if error.Success():
        print 'cstring: ', cstring
    else
        print 'error: ', error
    ") ReadCStringFromMemory;

    size_t
    ReadCStringFromMemory (addr_t addr, void *buf, size_t size, lldb::SBError &error);

    %feature("autodoc", "
    Reads an unsigned integer from memory given a byte size and an address. 
    Returns the unsigned integer that was read. Example:
    
    # Read a 4 byte unsigned integer from address 0x1000
    error = lldb.SBError()
    uint = ReadUnsignedFromMemory(0x1000, 4, error)
    if error.Success():
        print 'integer: %u' % uint
    else
        print 'error: ', error

    ") ReadUnsignedFromMemory;

    uint64_t
    ReadUnsignedFromMemory (addr_t addr, uint32_t byte_size, lldb::SBError &error);
    
    %feature("autodoc", "
    Reads a pointer from memory from an address and returns the value. Example:
    
    # Read a pointer from address 0x1000
    error = lldb.SBError()
    ptr = ReadPointerFromMemory(0x1000, error)
    if error.Success():
        print 'pointer: 0x%x' % ptr
    else
        print 'error: ', error
    
    ") ReadPointerFromMemory;
    
    lldb::addr_t
    ReadPointerFromMemory (addr_t addr, lldb::SBError &error);
    

    // Events
    static lldb::StateType
    GetStateFromEvent (const lldb::SBEvent &event);

    static bool
    GetRestartedFromEvent (const lldb::SBEvent &event);

    static size_t
    GetNumRestartedReasonsFromEvent (const lldb::SBEvent &event);
    
    static const char *
    GetRestartedReasonAtIndexFromEvent (const lldb::SBEvent &event, size_t idx);

    static lldb::SBProcess
    GetProcessFromEvent (const lldb::SBEvent &event);

    static bool
    EventIsProcessEvent (const lldb::SBEvent &event);

    lldb::SBBroadcaster
    GetBroadcaster () const;

    bool
    GetDescription (lldb::SBStream &description);

    uint32_t
    GetNumSupportedHardwareWatchpoints (lldb::SBError &error) const;

    uint32_t
    LoadImage (lldb::SBFileSpec &image_spec, lldb::SBError &error);
    
    lldb::SBError
    UnloadImage (uint32_t image_token);

    %feature("autodoc", "
    Return the number of different thread-origin extended backtraces
    this process can support as a uint32_t.
    When the process is stopped and you have an SBThread, lldb may be
    able to show a backtrace of when that thread was originally created,
    or the work item was enqueued to it (in the case of a libdispatch 
    queue).
    ") GetNumExtendedBacktraceTypes;
    
    uint32_t
    GetNumExtendedBacktraceTypes ();

    %feature("autodoc", "
    Takes an index argument, returns the name of one of the thread-origin 
    extended backtrace methods as a str.
    ") GetExtendedBacktraceTypeAtIndex;

    const char *
    GetExtendedBacktraceTypeAtIndex (uint32_t idx);

    %pythoncode %{
        def __get_is_alive__(self):
            '''Returns "True" if the process is currently alive, "False" otherwise'''
            s = self.GetState()
            if (s == eStateAttaching or 
                s == eStateLaunching or 
                s == eStateStopped or 
                s == eStateRunning or 
                s == eStateStepping or 
                s == eStateCrashed or 
                s == eStateSuspended):
                return True
            return False

        def __get_is_running__(self):
            '''Returns "True" if the process is currently running, "False" otherwise'''
            state = self.GetState()
            if state == eStateRunning or state == eStateStepping:
                return True
            return False

        def __get_is_running__(self):
            '''Returns "True" if the process is currently stopped, "False" otherwise'''
            state = self.GetState()
            if state == eStateStopped or state == eStateCrashed or state == eStateSuspended:
                return True
            return False

        class threads_access(object):
            '''A helper object that will lazily hand out thread for a process when supplied an index.'''
            def __init__(self, sbprocess):
                self.sbprocess = sbprocess
        
            def __len__(self):
                if self.sbprocess:
                    return int(self.sbprocess.GetNumThreads())
                return 0
        
            def __getitem__(self, key):
                if type(key) is int and key < len(self):
                    return self.sbprocess.GetThreadAtIndex(key)
                return None
        
        def get_threads_access_object(self):
            '''An accessor function that returns a modules_access() object which allows lazy thread access from a lldb.SBProcess object.'''
            return self.threads_access (self)
        
        def get_process_thread_list(self):
            '''An accessor function that returns a list() that contains all threads in a lldb.SBProcess object.'''
            threads = []
            accessor = self.get_threads_access_object()
            for idx in range(len(accessor)):
                threads.append(accessor[idx])
            return threads
        
        __swig_getmethods__["threads"] = get_process_thread_list
        if _newclass: threads = property(get_process_thread_list, None, doc='''A read only property that returns a list() of lldb.SBThread objects for this process.''')
        
        __swig_getmethods__["thread"] = get_threads_access_object
        if _newclass: thread = property(get_threads_access_object, None, doc='''A read only property that returns an object that can access threads by thread index (thread = lldb.process.thread[12]).''')

        __swig_getmethods__["is_alive"] = __get_is_alive__
        if _newclass: is_alive = property(__get_is_alive__, None, doc='''A read only property that returns a boolean value that indicates if this process is currently alive.''')

        __swig_getmethods__["is_running"] = __get_is_running__
        if _newclass: is_running = property(__get_is_running__, None, doc='''A read only property that returns a boolean value that indicates if this process is currently running.''')

        __swig_getmethods__["is_stopped"] = __get_is_running__
        if _newclass: is_stopped = property(__get_is_running__, None, doc='''A read only property that returns a boolean value that indicates if this process is currently stopped.''')

        __swig_getmethods__["id"] = GetProcessID
        if _newclass: id = property(GetProcessID, None, doc='''A read only property that returns the process ID as an integer.''')
        
        __swig_getmethods__["target"] = GetTarget
        if _newclass: target = property(GetTarget, None, doc='''A read only property that an lldb object that represents the target (lldb.SBTarget) that owns this process.''')
        
        __swig_getmethods__["num_threads"] = GetNumThreads
        if _newclass: num_threads = property(GetNumThreads, None, doc='''A read only property that returns the number of threads in this process as an integer.''')
        
        __swig_getmethods__["selected_thread"] = GetSelectedThread
        __swig_setmethods__["selected_thread"] = SetSelectedThread
        if _newclass: selected_thread = property(GetSelectedThread, SetSelectedThread, doc='''A read/write property that gets/sets the currently selected thread in this process. The getter returns a lldb.SBThread object and the setter takes an lldb.SBThread object.''')
        
        __swig_getmethods__["state"] = GetState
        if _newclass: state = property(GetState, None, doc='''A read only property that returns an lldb enumeration value (see enumerations that start with "lldb.eState") that represents the current state of this process (running, stopped, exited, etc.).''')
        
        __swig_getmethods__["exit_state"] = GetExitStatus
        if _newclass: exit_state = property(GetExitStatus, None, doc='''A read only property that returns an exit status as an integer of this process when the process state is lldb.eStateExited.''')
        
        __swig_getmethods__["exit_description"] = GetExitDescription
        if _newclass: exit_description = property(GetExitDescription, None, doc='''A read only property that returns an exit description as a string of this process when the process state is lldb.eStateExited.''')
        
        __swig_getmethods__["broadcaster"] = GetBroadcaster
        if _newclass: broadcaster = property(GetBroadcaster, None, doc='''A read only property that an lldb object that represents the broadcaster (lldb.SBBroadcaster) for this process.''')
    %}

};

}  // namespace lldb
