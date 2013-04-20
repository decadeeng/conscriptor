Summary:	conscriptor
Group:		User Interface/Desktops
License:	Decade Engineering
URL:		http://decade-engineering.com
Packager:       James
Source:		%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
conscriptor font editor

%prep

%setup

%build
  qmake conscriptor.pro
  make

%install
  mkdir -p $RPM_BUILD_ROOT/usr/share/conscriptor/doc
  mkdir -p $RPM_BUILD_ROOT/usr/share/conscriptor/BDF_files
  mkdir -p $RPM_BUILD_ROOT/usr/share/conscriptor/BOB-4_Font_Files
  mkdir -p $RPM_BUILD_ROOT/usr/bin
  cp BDF\ files/* $RPM_BUILD_ROOT/usr/share/conscriptor/BDF_files
  cp BOB-4\ Font\ Files/* $RPM_BUILD_ROOT/usr/share/conscriptor/BOB-4_Font_Files
  cp -a doc $RPM_BUILD_ROOT/usr/share/conscriptor
  cp conscriptor/conscriptor $RPM_BUILD_ROOT/usr/bin

%clean
  test -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != "/" && rm -rf $RPM_BUILD_ROOT

%files
/usr/share/conscriptor
/usr/bin/conscriptor

%changelog

