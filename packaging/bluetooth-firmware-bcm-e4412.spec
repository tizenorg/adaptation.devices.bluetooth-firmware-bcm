Name:       bluetooth-firmware-bcm-e4412
Summary:    firmware and tools for bluetooth
Version:    0.2.28
Release:    1
Group:      TO_BE_FILLED
License:    TO_BE_FILLED
Source0:    bluetooth-firmware-bcm-%{version}.tar.gz

BuildRequires:  pkgconfig(vconf)
BuildRequires:  cmake

%description
firmware and tools for bluetooth


%prep
%setup -q -n bluetooth-firmware-bcm-%{version}

%build
cmake ./ -DCMAKE_INSTALL_PREFIX=%{_prefix} -DPLUGIN_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

cp -rf %{buildroot}%{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh  %{buildroot}%{_prefix}/etc/bluetooth/bt-dev-start.sh

%files
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.0079.0081.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
%exclude %{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%exclude %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh
%exclude %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh
