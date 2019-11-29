#include <fstream>
#include <iostream>
#include <string>

#include "breakpoint/breakpoint.hpp"
#include "util/simple_options.hpp"
#include "util/sndfile_utils.hpp" // TODO should not need

static SndfileErr process_breakpoints_file( const std::string & infile ) noexcept {
    auto result = breakpoint::parse_breakpoints( infile );
    auto * err = std::get_if<breakpoint::parse_error>( &result );
    if ( err ) {
        std::cout << "Error " << to_string( err->code ) << " on line: " << err->line << std::endl;
        return SndfileErr::CouldNotOpen;
    }

    auto & points = *std::get_if<std::vector<breakpoint::point>>( &result );
    for ( auto && pt : points ) {
        std::cout << "Point: " << pt.time_secs << " @ " << pt.value << std::endl;
    }

    auto && max_pt = breakpoint::max_point( begin( points ), end( points ) );
    std::cout << "\nMax point: " << max_pt.time_secs << " @ " << max_pt.value << std::endl;
    return SndfileErr::Success;
}

int main( int argc, char ** argv ) {
    simple_options::options opts{ "bkpts_driver" };
    opts.basic_option( "help,h", "Print description and exit" )
        .positional( "file", "Input file" )
        .parse( argc, argv );

    checked_invoke( opts, std::array{"file"}, &process_breakpoints_file );
}
