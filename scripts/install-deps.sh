#!/usr/bin/env bash

set -e

SKIPCMAKE=false
SKIPMARIADB=false
SKIPSQLITE=false
SKIPSQLSERVER=false
SKIPJSON=false
HELP=false

function parse_options {
  while [[ $# -gt 0 ]]
  do
    OPTION="${1}"
    case $OPTION in
      --skip-cmake)           SKIPCMAKE=true          ;;
      --skip-mariadb)         SKIPMARIADB=true        ;;
      --skip-sqlite)          SKIPSQLITE=true         ;;
      --skip-odbc)            SKIPSQLSERVER=true      ;;
      --skip-json)            SKIPJSON=true           ;;
      *)                      HELP=true               ; break ;;
    esac
    shift
  done
}

parse_options "$@"

if [ "$HELP" == "true" ]; then
    echo "All dependencies downloaded by default. Options available: "
    echo "  --skip-cmake             Skip CMake download"
    echo "  --skip-mariadb           Skip MariaDB download"
    echo "  --skip-sqlite            Skip SQLite download"
    echo "  --skip-odbc              Skip ODBC (SQL Server) download"
    echo "  --skip-json              Skip JSON library download"
    echo -e "\nThis script must be run as sudo."

    exit 0
fi

if (( $EUID != 0 )); then
    >&2 echo "This script must be run as root, as it installs dependencies via the system package manager. Please run using sudo."
    exit -1
fi

set +e

PLATFORM_NAME=$(grep -qi ubuntu /etc/os-release && echo 'ubuntu')
PLATFORM_VERSION=$(grep -oPi 'VERSION_ID=\K"(.+)"' /etc/os-release | sed -e 's/"//g')
if [ -z "$PLATFORM_NAME" ]; then
    PLATFORM_NAME=$(egrep -qi "(rhel|centos)" /etc/os-release && echo 'rhel')

    if [ -z "$PLATFORM_NAME" ]; then
        >&2 echo "Unsupported platform detected! Use Ubuntu/rhel"
        exit -1
    fi
fi

SUPPORTED_VERSIONS=" 14.04 16.04 18.04 18.10 19.04 20.04 22.04 6 7 8 "
VERSION_IS_SUPPORTED=$(echo $SUPPORTED_VERSIONS | grep -oP " $PLATFORM_VERSION ")
if [ -z "$PLATFORM_VERSION" ]; then
    >&2 echo "Unable to detect platform version! Check that /etc/os-release has a VERSION_ID= set."
    exit -1
elif [ -z "$VERSION_IS_SUPPORTED" ]; then
    >&2 echo "Platform version is unsupported! Detected version: $PLATFORM_VERSION"
    >&2 echo "Supported versions: $SUPPORTED_VERSIONS"
    exit -1
fi

set -e

echo "Detected platform: $PLATFORM_NAME $PLATFORM_VERSION"

PACKAGES="g++ make"
if [ "$PLATFORM_NAME" == "rhel" ]; then
    PACKAGES="gcc-c++ make"
fi

if [ "$SKIPMARIADB" == "false" ]; then
    if [ "$PLATFORM_NAME" == "ubuntu" ]; then
        PACKAGES="$PACKAGES libmariadb-dev"
    elif [ "$PLATFORM_NAME" == "rhel" ]; then
        PACKAGES="$PACKAGES mariadb-devel"
    fi
fi

if [ "$SKIPSQLITE" == "false" ]; then
    if [ "$PLATFORM_NAME" == "ubuntu" ]; then
        PACKAGES="$PACKAGES libsqlite3-dev"
    elif [ "$PLATFORM_NAME" == "rhel" ]; then
        PACKAGES="$PACKAGES sqlite-devel"
    fi
fi

if [ "$SKIPSQLSERVER" == "false" ]; then
    if [ "$PLATFORM_NAME" == "ubuntu" ]; then
        if [ ! -f '/etc/apt/sources.list.d/mssql-release.list' ]; then
            curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
            curl https://packages.microsoft.com/config/ubuntu/${PLATFORM_VERSION}/prod.list > /etc/apt/sources.list.d/mssql-release.list
        fi
        apt-get update > /dev/null
        PACKAGES="$PACKAGES msodbcsql17 unixodbc-dev"
    elif [ "$PLATFORM_NAME" == "rhel" ]; then
        if [ ! -f '/etc/yum.repos.d/mssql-release.repo' ]; then
            curl https://packages.microsoft.com/config/rhel/${PLATFORM_VERSION}/prod.repo > /etc/yum.repos.d/mssql-release.repo
        fi
        yum remove -y unixODBC-utf16 unixODBC-utf16-devel > /dev/null
        PACKAGES="$PACKAGES msodbcsql17 unixODBC-devel"
    fi
fi

if [ -z "$PACKAGES" ]; then
    >&2 echo "No packages were selected for installation"
    exit -1
fi

echo "Installing packages: $PACKAGES"

if [ "$PLATFORM_NAME" == "ubuntu" ]; then
    apt-get update > /dev/null
    ACCEPT_EULA=Y apt-get install -y $PACKAGES
elif [ "$PLATFORM_NAME" == "rhel" ]; then
    yum makecache fast > /dev/null
    ACCEPT_EULA=Y yum install -y $PACKAGES
fi

if [ "$SKIPCMAKE" == "false" ]; then
    echo "Installing cmake..."
    if [ "$PLATFORM_NAME" == "ubuntu" ]; then
        apt-get remove -y cmake > /dev/null
    elif [ "$PLATFORM_NAME" == "rhel" ]; then
        yum remove -y cmake > /dev/null
    fi
    wget -q https://github.com/Kitware/CMake/releases/download/v3.16.0/cmake-3.16.0-Linux-x86_64.sh
    chmod +x ./cmake-3.16.0-Linux-x86_64.sh
    ./cmake-3.16.0-Linux-x86_64.sh --skip-license --prefix=/usr
    rm cmake-3.16.0-Linux-x86_64.sh
fi

if [ "$SKIPJSON" == "false" ]; then
    echo "Installing json library dependency"

    if [ ! -d ../json ]; then
        mkdir ../json
    fi
    JSON_VERSION=3.9.1
    wget -q "https://raw.githubusercontent.com/nlohmann/json/v$JSON_VERSION/single_include/nlohmann/json.hpp"
    mv json.hpp ../json
fi
