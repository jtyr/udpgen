Name:		udpgen
Version:	0.1
Release:	2.%{dist}.cern
Summary:	Program which allows to send UDP packets into the multicast network.
Packager:	Jiri Tyr

Group:		Productivity/Networking
License:	GPLv3
URL:		http://www.cern.ch
Source:		%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires:	gcc
BuildRequires:	make
BuildRequires:	glibc-devel
BuildRequires:	perl(ExtUtils::Manifest)


%description
UDPGen is a simple UDP packet generator which allows to send UDP packets into
the multicast network.


%prep
%setup -q -n %{name}-%{version}


%build
%{__make} %{?_smp_mflags}


%install
[ "%{buildroot}" != / ] && %{__rm} -rf "%{buildroot}"
%{__make} install DESTDIR=%{buildroot}


%clean
[ "%{buildroot}" != / ] && %{__rm} -rf "%{buildroot}"


%files
%defattr(-,root,root,-)
# For license text(s), see the perl package.
%doc Changes README
%{_bindir}/udpgen


%changelog
* Fri Nov 13 2009 Jiri Tyr <jiri.tyr at cern.ch>
- First build.
