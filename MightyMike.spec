#
# spec file for package MightyMike by <https://www.notabug.org/tux_peng>
#
# Mighty Mike is a registered trademark of Pangea Software, Inc.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/jorio/MightyMike/issues
#
%global debug_package %{nil}


Name:           MightyMike
Version:        3.0.1
Release:        0
Summary:        It's up to MightyMike to save the innocent fuzzy bunnies
License:        CC-BY-NC-SA-4.0
Group:          Amusements/Games/3D/Other
URL:            https://github.com/jorio/MightyMike/
Source:         MightyMike-%{version}.tar.gz
BuildRequires:  SDL2-devel
BuildRequires:  cmake >= 3.13
BuildRequires:  glibc >= 2.2.5
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(glu)
BuildRequires:  gcc-c++

%description

When the lights of the Toy Mart go out, the adventure begins: it's up to MightyMike to save the innocent fuzzy bunnies from evil clowns, crazed robots and bone-hurling cavemen. Use an arsenal of weapons from suction cups to gum ball blasters to blaze your way through 15 levels of toy shop terror.


%prep
%setup -q


%build
# FIXME: you should use the %%cmake macros
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
# FIXME: you should use the %%cmake macros
cmake --build build-release

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}/%{name}
mkdir -p %{buildroot}%{_datadir}/{pixmaps,applications/}
echo -e '#!/usr/bin/sh\n\ncd %{_libdir}/%{name}/;./%{name}' > '%{buildroot}%{_bindir}/%{name}.sh'
chmod +x %{buildroot}%{_bindir}/%{name}.sh
mv build-release/{Data,%{name}} %{buildroot}%{_libdir}/%{name}
mv cmake/%{name}32.png %{buildroot}%{_datadir}/pixmaps/%{name}.png
mv %{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop


%files
%license {LICENSE.md,docs,README.md}
%{_bindir}/%{name}.sh
%{_libdir}/%{name}/
%{_datadir}/pixmaps/%{name}.png
%{_datadir}/applications/%{name}.desktop

%changelog
