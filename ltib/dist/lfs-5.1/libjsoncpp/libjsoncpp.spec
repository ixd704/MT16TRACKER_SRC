%define pfx /opt/freescale/rootfs/%{_target_cpu}

Summary         : JSON C++ Library with custom modifications
Name            : libjsoncpp
Version         : 1.0
Release         : 1
License         : GPL
Vendor          : Gadgeon
Packager        : Sudheer Divakaran
Group           : Applications/System
URL             : http://www.gadgeon.com
Source          : %{name}-%{version}.tar.bz2
BuildRoot       : %{_tmppath}/%{name}
Prefix          : %{pfx}

%Description
%{summary}


%Prep
%setup

%Build
BUILDCC=gcc
BUILDSTRIP=strip
make HOSTCC="$BUILDCC" HOSTSTRIP="$BUILDSTRIP"

%Install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{pfx}/%{_prefix}/bin
make DESTDIR=$RPM_BUILD_ROOT/%{pfx}/%{_prefix} install


%Clean
rm -rf $RPM_BUILD_ROOT

%Files
%defattr(-,root,root)
%{pfx}/*
