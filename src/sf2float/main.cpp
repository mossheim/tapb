#include <iostream>
#include <string>
#include <vector>

#include "util/simple_options.hpp"
#include "util/sndfile_utils.hpp"

#include "sndfile.hh"

SndfileErr do_copy_impl( SndfileHandle & from,
                         SndfileHandle & to,
                         const size_t bufsize,
                         std::vector<float> & floats ) {
    sf_count_t read = 0;
    sf_count_t total_written = 0;
    while ( ( read = from.readf( floats.data(), bufsize ) ) ) {
        auto written = to.writef( floats.data(), read );
        if ( written < read ) {
            std::cout << "Error while writing (" << written << " written): " << to.strError()
                      << std::endl;
            return SndfileErr::BadWrite;
        }

        total_written += written;
    }

    if ( total_written != from.frames() ) {
        std::cout << "Could not read entire file: " << from.strError() << std::endl;
        std::cout << "Read " << from.frames() << " | Wrote " << to.frames() << std::endl;
        return SndfileErr::BadRead;
    }

    return SndfileErr::Success;
}

SndfileErr do_copy_repeated( SndfileHandle & from,
                             SndfileHandle & to,
                             const size_t bufsize,
                             const size_t repeats ) {
    std::vector<float> floats( from.channels() * bufsize );

    for ( auto i = 0ul; i < repeats; ++i ) {
        auto const result = do_copy_impl( from, to, bufsize, floats );
        if ( SndfileErr::Success != result ) {
            return result;
        }

        from.seek( 0, 0 );
    }

    return SndfileErr::Success;
}

SndfileErr do_copy( const std::string & from_path,
                    const std::string & to_path,
                    const size_t bufsize,
                    const size_t repeats ) {
    SndfileHandle from{ from_path, SFM_READ };
    if ( from.error() != SF_ERR_NO_ERROR ) {
        std::cout << "Could not open read file: " << from_path << std::endl;
        return SndfileErr::CouldNotOpen;
    }

    SndfileHandle to{ to_path, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, from.channels(),
                      from.samplerate() };
    if ( to.error() != SF_ERR_NO_ERROR ) {
        std::cout << "Could not open write file: " << to_path << std::endl;
        return SndfileErr::CouldNotOpen;
    }

#ifndef NDEBUG
    std::cout << from << "\n" << to << "\n";
#endif

    return do_copy_repeated( from, to, bufsize, repeats );
}

int main( int argc, char ** argv ) {
    simple_options::options opts{ "sf2float" };
    size_t bufsize, repeats;
    opts.basic_option( "help,h", "Print description and exit" )
        .basic_option( "bufsize,b", "Buffer size in frames",
                       simple_options::defaulted_value( &bufsize, 1 ) )
        .basic_option( "repeats,r", "Number of times to repeat",
                       simple_options::defaulted_value( &repeats, 1 ) )
        .positional( "input", "Input file" )
        .positional( "output", "Output file" )
        .parse( argc, argv );

    if ( opts.has( "help" ) ) {
        std::cout << opts;
        return 0;
    }

    if ( opts.has( "input" ) && opts.has( "output" ) ) {
        auto && input = opts["input"].as<std::string>();
        auto && output = opts["output"].as<std::string>();
        if ( do_copy( input, output, bufsize, repeats ) == SndfileErr::Success ) {
            return 0;
        } else {
            std::cout << "Copy failed." << std::endl;
        }
    } else {
        std::cout << opts;
        return 1;
    }

    return 0;
}
