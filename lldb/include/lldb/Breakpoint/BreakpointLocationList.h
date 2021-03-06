//===-- BreakpointLocationList.h --------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_BreakpointLocationList_h_
#define liblldb_BreakpointLocationList_h_

// C Includes
// C++ Includes
#include <vector>
#include <map>
// Other libraries and framework includes
// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Core/Address.h"
#include "lldb/Host/Mutex.h"

namespace lldb_private {

//----------------------------------------------------------------------
/// @class BreakpointLocationList BreakpointLocationList.h "lldb/Breakpoint/BreakpointLocationList.h"
/// @brief This class is used by Breakpoint to manage a list of breakpoint locations,
//  each breakpoint location in the list
/// has a unique ID, and is unique by Address as well.
//----------------------------------------------------------------------

class BreakpointLocationList
{
// Only Breakpoints can make the location list, or add elements to it.
// This is not just some random collection of locations.  Rather, the act of adding the location
// to this list sets its ID, and implicitly all the locations have the same breakpoint ID as
// well.  If you need a generic container for breakpoint locations, use BreakpointLocationCollection.
friend class Breakpoint;

public:
    virtual 
    ~BreakpointLocationList();

    //------------------------------------------------------------------
    /// Standard "Dump" method.  At present it does nothing.
    //------------------------------------------------------------------
    void
    Dump (Stream *s) const;

    //------------------------------------------------------------------
    /// Returns a shared pointer to the breakpoint location at address
    /// \a addr - const version.
    ///
    /// @param[in] addr
    ///     The address to look for.
    ///
    /// @result
    ///     A shared pointer to the breakpoint.  May contain a NULL
    ///     pointer if the breakpoint doesn't exist.
    //------------------------------------------------------------------
    const lldb::BreakpointLocationSP
    FindByAddress (const Address &addr) const;

    //------------------------------------------------------------------
    /// Returns a shared pointer to the breakpoint location with id
    /// \a breakID, const version.
    ///
    /// @param[in] breakID
    ///     The breakpoint location ID to seek for.
    ///
    /// @result
    ///     A shared pointer to the breakpoint.  May contain a NULL
    ///     pointer if the breakpoint doesn't exist.
    //------------------------------------------------------------------
    lldb::BreakpointLocationSP
    FindByID (lldb::break_id_t breakID) const;

    //------------------------------------------------------------------
    /// Returns the breakpoint location id to the breakpoint location
    /// at address \a addr.
    ///
    /// @param[in] addr
    ///     The address to match.
    ///
    /// @result
    ///     The ID of the breakpoint location, or LLDB_INVALID_BREAK_ID.
    //------------------------------------------------------------------
    lldb::break_id_t
    FindIDByAddress (const Address &addr);

    //------------------------------------------------------------------
    /// Returns a breakpoint location list of the breakpoint locations
    /// in the module \a module.  This list is allocated, and owned by
    /// the caller.
    ///
    /// @param[in] module
    ///     The module to seek in.
    ///
    /// @param[in]
    ///     A breakpoint collection that gets any breakpoint locations
    ///     that match \a module appended to.
    ///
    /// @result
    ///     The number of matches
    //------------------------------------------------------------------
    size_t
    FindInModule (Module *module,
                  BreakpointLocationCollection& bp_loc_list);

    //------------------------------------------------------------------
    /// Returns a shared pointer to the breakpoint location with
    /// index \a i.
    ///
    /// @param[in] i
    ///     The breakpoint location index to seek for.
    ///
    /// @result
    ///     A shared pointer to the breakpoint.  May contain a NULL
    ///     pointer if the breakpoint doesn't exist.
    //------------------------------------------------------------------
    lldb::BreakpointLocationSP
    GetByIndex (size_t i);

    //------------------------------------------------------------------
    /// Returns a shared pointer to the breakpoint location with index
    /// \a i, const version.
    ///
    /// @param[in] i
    ///     The breakpoint location index to seek for.
    ///
    /// @result
    ///     A shared pointer to the breakpoint.  May contain a NULL
    ///     pointer if the breakpoint doesn't exist.
    //------------------------------------------------------------------
    const lldb::BreakpointLocationSP
    GetByIndex (size_t i) const;

    //------------------------------------------------------------------
    /// Removes all the locations in this list from their breakpoint site
    /// owners list.
    //------------------------------------------------------------------
    void
    ClearAllBreakpointSites ();

    //------------------------------------------------------------------
    /// Tells all the breakopint locations in this list to attempt to
    /// resolve any possible breakpoint sites.
    //------------------------------------------------------------------
    void
    ResolveAllBreakpointSites ();

    //------------------------------------------------------------------
    /// Returns the number of breakpoint locations in this list with
    /// resolved breakpoints.
    ///
    /// @result
    ///     Number of qualifying breakpoint locations.
    //------------------------------------------------------------------
    size_t
    GetNumResolvedLocations() const;

    //------------------------------------------------------------------
    /// Returns the number hit count of all locations in this list.
    ///
    /// @result
    ///     Hit count of all locations in this list.
    //------------------------------------------------------------------
    uint32_t
    GetHitCount () const;

    //------------------------------------------------------------------
    /// Enquires of the breakpoint location in this list with ID \a
    /// breakID whether we should stop.
    ///
    /// @param[in] context
    ///     This contains the information about this stop.
    ///
    /// @param[in] breakID
    ///     This break ID that we hit.
    ///
    /// @return
    ///     \b true if we should stop, \b false otherwise.
    //------------------------------------------------------------------
    bool
    ShouldStop (StoppointCallbackContext *context,
                lldb::break_id_t breakID);

    //------------------------------------------------------------------
    /// Returns the number of elements in this breakpoint location list.
    ///
    /// @result
    ///     The number of elements.
    //------------------------------------------------------------------
    size_t
    GetSize() const
    {
        return m_locations.size();
    }

    //------------------------------------------------------------------
    /// Print a description of the breakpoint locations in this list to
    /// the stream \a s.
    ///
    /// @param[in] s
    ///     The stream to which to print the description.
    ///
    /// @param[in] level
    ///     The description level that indicates the detail level to
    ///     provide.
    ///
    /// @see lldb::DescriptionLevel
    //------------------------------------------------------------------
    void
    GetDescription (Stream *s,
                    lldb::DescriptionLevel level);

protected:

    //------------------------------------------------------------------
    /// This is the standard constructor.
    ///
    /// It creates an empty breakpoint location list. It is protected
    /// here because only Breakpoints are allowed to create the
    /// breakpoint location list.
    //------------------------------------------------------------------
    BreakpointLocationList(Breakpoint &owner);

    //------------------------------------------------------------------
    /// Add the breakpoint \a bp_loc_sp to the list.
    ///
    /// @param[in] bp_sp
    ///     Shared pointer to the breakpoint location that will get
    ///     added to the list.
    ///
    /// @result
    ///     Returns breakpoint location id.
    //------------------------------------------------------------------
    lldb::BreakpointLocationSP
    Create (const Address &addr);
    
    void
    StartRecordingNewLocations(BreakpointLocationCollection &new_locations);
    
    void
    StopRecordingNewLocations();
    
    lldb::BreakpointLocationSP
    AddLocation (const Address &addr,
                 bool *new_location = NULL);

    bool
    RemoveLocation (const lldb::BreakpointLocationSP &bp_loc_sp);
    
    void
    RemoveInvalidLocations (const ArchSpec &arch);

    typedef std::vector<lldb::BreakpointLocationSP> collection;
    typedef std::map<lldb_private::Address,
                     lldb::BreakpointLocationSP,
                     Address::ModulePointerAndOffsetLessThanFunctionObject> addr_map;

    Breakpoint &m_owner;
    collection m_locations;
    addr_map m_address_to_location;
    mutable Mutex m_mutex;
    lldb::break_id_t m_next_id;
    BreakpointLocationCollection *m_new_location_recorder;
};

} // namespace lldb_private

#endif  // liblldb_BreakpointLocationList_h_
