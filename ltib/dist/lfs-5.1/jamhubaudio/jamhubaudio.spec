%define pfx /opt/freescale/rootfs/%{_target_cpu}
%define __os_install_post %{nil}

Summary         : Jamhub capture and encode package
Name            : jamhubaudio
Version         : 1.1
Release         : 2
License         : Public Domain, not copyrighted
Vendor          : EnvidTechnologies
Packager        : Pavel B
Group           : Applications/Test
Source          : %{name}-%{version}.tar.bz2
BuildRoot       : %{_tmppath}/%{name}
Prefix          : %{pfx}

%Description
%{summary}

%Prep
%setup

%Build
make prefix=%{_prefix}

%Install
rm -rf $RPM_BUILD_ROOT
make install prefix=%{_prefix} DESTDIR=$RPM_BUILD_ROOT/%{pfx}

%Clean
rm -rf $RPM_BUILD_ROOT

%Files
%defattr(-,root,root)
%{pfx}/*

