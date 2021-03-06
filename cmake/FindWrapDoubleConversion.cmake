# We can't create the same interface imported target multiple times, CMake will complain if we do
# that. This can happen if the find_package call is done in multiple different subdirectories.
if(TARGET WrapDoubleConversion::WrapDoubleConversion)
    set(WrapDoubleConversion_FOUND ON)
    return()
endif()

add_library(WrapDoubleConversion::WrapDoubleConversion INTERFACE IMPORTED)

find_package(double-conversion)
if (double-conversion_FOUND)
    include(FeatureSummary)
    set_package_properties(double-conversion PROPERTIES TYPE REQUIRED)
    target_link_libraries(WrapDoubleConversion::WrapDoubleConversion
                          INTERFACE double-conversion::double-conversion)
    set(WrapDoubleConversion_FOUND 1)
    return()
endif()

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
#include <stdio.h>
#include <locale.h>

int main(int argc, char *argv[]) {
    _locale_t invalidLocale = NULL;
    double a = 3.14;
    const char *format = \"invalid format\";
    _sscanf_l(argv[0], invalidLocale, format, &a, &argc);
    _snprintf_l(argv[0], 1, invalidLocale, format, a);
}" HAVE__SPRINTF_L)

check_cxx_source_compiles("
#include <stdio.h>
#include <xlocale.h>

int main(int argc, char *argv[]) {
    locale_t invalidLocale = NULL;
    double a = 3.14;
    const char *format = \"invalid format\";
    snprintf_l(argv[0], 1, invalidLocale, format, a);
    sscanf_l(argv[0], invalidLocale, format, &a, &argc);
    return 0;
}" HAVE_SPRINTF_L)

# In a static build, we need to find the package to bring the target into scope.
find_package(QtDoubleConversion QUIET)

if (HAVE__SPRINTF_L OR HAVE_SPRINTF_L)
    target_compile_definitions(WrapDoubleConversion::WrapDoubleConversion
                               INTERFACE QT_NO_DOUBLECONVERSION)
    set(WrapDoubleConversion_FOUND 1)
elseif(TARGET QtDoubleConversion)
    # If a Config package wasn't found, and the C++ library doesn't contain the necessary functions,
    # use the library bundled with Qt.
    target_link_libraries(WrapDoubleConversion::WrapDoubleConversion INTERFACE QtDoubleConversion)
    set(WrapDoubleConversion_FOUND 1)
else()
    set(WrapDoubleConversion_FOUND 0)
endif()
