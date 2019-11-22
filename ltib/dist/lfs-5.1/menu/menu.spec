%define pfx /opt/freescale/rootfs/%{_target_cpu}

Summary         : Menu
Name            : menu
Version         : 1.0
Release         : 1
License         : GPL
Vendor          : EnvidTechnologies
Packager        : Pavel B
Group           : Applications/System
URL             : http://ya.ru
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
make -C src HOSTCC="$BUILDCC" HOSTSTRIP="$BUILDSTRIP"

%Install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{pfx}/%{_prefix}/bin
make -C src DESTDIR=$RPM_BUILD_ROOT/%{pfx}/%{_prefix} install


%Clean
rm -rf $RPM_BUILD_ROOT

%Files
%defattr(-,root,root)
%{pfx}/*
