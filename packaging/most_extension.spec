Name:       most_extension
Summary:    A collection of IVI software
Version:    0.0.1
Release:    1
Group:      Libraries/System
License:    ASL 2.0
URL:        http://www.tizen.org2
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  python
BuildRequires:  desktop-file-utils

BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eldbus)

Requires:       ibus
Requires:       ibus-hangul
Requires:       ibus-libpinyin


%global plugin_list extension_common most 

%description
A collection of IVI software

%prep

%setup -q -n %{name}-%{version}

%build
for plugin in %{plugin_list}; do
    make -C ${plugin}
done

%install
for plugin in %{plugin_list}; do
    make -C ${plugin} install DESTDIR=%{buildroot} PREFIX=%{_prefix}
done

%post
sudo systemctl enable MOST.service 
sudo chmod a+x /etc/most

%postun
sudo rm -fr /etc/most

# The MOSTinit, MOST.service and MOST-ready.path are part of the systemd method to execute
# MOSTinit at boot up. 
%files
%{_prefix}/lib/tizen-extensions-crosswalk/libmost.so
%{_prefix}/bin/MOSTinit
%{_prefix}/lib/systemd/system/MOST.service
%{_prefix}/lib/systemd/system/MOST-ready.path
%{_prefix}/../etc/most/conf.NDIS
%{_prefix}/../etc/most/conf.NUC
%{_prefix}/../etc/most/conf.VTC1010
